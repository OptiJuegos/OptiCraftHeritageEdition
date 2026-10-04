// DsHttpDownload.cpp -- libcurl file download, driven by the game loop.
// See DsHttpDownload.h for why this is curl instead of http:C.
#ifdef CTR_PLATFORM

#include "3ds/qr/DsHttpDownload.h"

// curl's multi.h declares fd_set-taking entry points but does not include
// the header itself (known 3ds portlibs quirk); provide it first.
#include <sys/select.h>

#include <curl/curl.h>

#include <cstdlib>
#include <cstring>

#include "platform/Log.h"

#if defined(CTR_ENABLE_NETWORK)
#include "3ds/DsNetwork.h"
#else
#include <3ds.h>
#include <malloc.h>
#endif

namespace
{
// The whole point of a QR download is skins, packs and mods; anything bigger
// than this is not one, and streaming it to SD anyway would just waste the
// card's space behind a bar that never seems to end.
constexpr curl_off_t kMaxDownloadBytes = 96 * 1024 * 1024;

// A stalled server must not own the menu: give up the whole transfer if
// less than a byte a second moves for this long, and stop waiting for the
// TCP connect sooner than curl's own default (which is minutes).
constexpr long kConnectTimeoutSecs = 20;
constexpr long kLowSpeedLimit = 1;
constexpr long kLowSpeedTimeSecs = 30;

std::string describeCurlFailure(CURLcode code)
{
	return "the server or the connection failed (" +
	       std::to_string(static_cast<int>(code)) + ": " +
	       curl_easy_strerror(code) + ")";
}

std::size_t writeToFile(char *ptr, std::size_t size, std::size_t nmemb, void *userdata)
{
	std::FILE *file = static_cast<std::FILE *>(userdata);
	const std::size_t bytes = size * nmemb;
	return std::fwrite(ptr, 1, bytes, file);
}

// Capture Content-Disposition's filename from the FINAL response (curl
// reports every redirect hop's headers too; keeping the last one is exactly
// right, the direct file server is the one that knows the real name).
std::size_t captureHeader(char *buffer, std::size_t size, std::size_t nitems, void *userdata)
{
	const std::size_t bytes = size * nitems;
	std::string header(buffer, bytes);
	std::string *suggested = static_cast<std::string *>(userdata);

	std::string lower = header;
	for (char &c : lower)
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c - 'A' + 'a');
	if (lower.compare(0, 19, "content-disposition:") != 0)
		return bytes;

	const std::size_t marker = lower.find("filename=");
	if (marker == std::string::npos)
		return bytes;
	std::string name = header.substr(marker + 9);
	if (!name.empty() && name.front() == '"')
		name.erase(name.begin());
	const std::size_t quote = name.find('"');
	if (quote != std::string::npos)
		name = name.substr(0, quote);
	const std::size_t end = name.find_first_of(";\r\n");
	if (end != std::string::npos)
		name = name.substr(0, end);
	if (!name.empty())
		*suggested = name;
	return bytes;
}

bool startsWithHttp(const std::string &url)
{
	std::string head = url.substr(0, 8);
	for (char &c : head)
		if (c >= 'A' && c <= 'Z')
			c = static_cast<char>(c - 'A' + 'a');
	return head.compare(0, 7, "http://") == 0 || head.compare(0, 8, "https://") == 0;
}

// curl rides libctru's sockets, which need the soc:U service up -- the old
// httpc path never did (it owns its sockets inside the service), which is
// why the downloader has to bring it up explicitly here. With networking
// enabled the shared DsNetwork block (also used by multiplayer) does it;
// otherwise a private segment is taken for the download's lifetime.
#if !defined(CTR_ENABLE_NETWORK)
u32 *socSegment = nullptr;
#endif

bool ensureSockets(std::string &outError)
{
#if defined(CTR_ENABLE_NETWORK)
	// A helpful error before the DNS lookup would answer "couldn't resolve
	// host" on a console whose radio is simply off.
	const std::string wifiError = DsNetwork::wifiPreflightError();
	if (!wifiError.empty())
	{
		outError = wifiError;
		return false;
	}
	if (DsNetwork::initialize())
		return true;
	outError = "The console's network service did not start";
	return false;
#else
	if (socSegment != nullptr)
		return true;
	socSegment = static_cast<u32 *>(memalign(0x1000, 0x100000));
	if (socSegment == nullptr || socInit(socSegment, 0x100000) != 0)
	{
		free(socSegment);
		socSegment = nullptr;
		outError = "The console's network service did not start";
		return false;
	}
	return true;
#endif
}
} // namespace

DsHttpDownload::~DsHttpDownload()
{
	cancel();
}

bool DsHttpDownload::begin(const std::string &url, const std::string &destPathValue,
                           std::string &outError)
{
	outError.clear();
	cancel();

	if (!startsWithHttp(url))
	{
		outError = "The code does not contain a http(s) web address";
		return false;
	}

	if (!ensureSockets(outError))
		return false;

	if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
	{
		outError = "The network stack could not start";
		return false;
	}
	easy = curl_easy_init();
	multi = curl_multi_init();
	if (easy == nullptr || multi == nullptr)
	{
		outError = "The network stack could not start";
		closeHandles();
		return false;
	}

	file = std::fopen(destPathValue.c_str(), "wb");
	if (file == nullptr)
	{
		outError = "The destination file could not be created";
		closeHandles();
		return false;
	}
	destPath = destPathValue;
	received = 0;
	total = 0;
	suggestedName.clear();

	curl_easy_setopt(easy, CURLOPT_URL, url.c_str());
	// Redirects (a Drive uc?export=download link hops to
	// drive.usercontent.google.com, shorteners hop once more), bounded so a
	// redirect loop fails instead of spinning.
	curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(easy, CURLOPT_MAXREDIRS, 5L);
	curl_easy_setopt(easy, CURLOPT_USERAGENT, "OptiCraft-Heritage/3DS");
	// No CA bundle ship with the console build: curl is linked against
	// mbedTLS with no system cert store, so chain verification has nothing
	// to verify against and would fail every HTTPS host. Verification off
	// is what the other 3DS downloaders (Anemone3DS) settled on too -- the
	// native ssl:C stack this replaces never verified these servers either.
	curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(easy, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT, kConnectTimeoutSecs);
	curl_easy_setopt(easy, CURLOPT_LOW_SPEED_LIMIT, kLowSpeedLimit);
	curl_easy_setopt(easy, CURLOPT_LOW_SPEED_TIME, kLowSpeedTimeSecs);
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &writeToFile);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION, &captureHeader);
	curl_easy_setopt(easy, CURLOPT_HEADERDATA, &suggestedName);

	if (curl_multi_add_handle(multi, easy) != CURLM_OK)
	{
		outError = "The request could not be sent";
		closeHandles();
		return false;
	}
	multiHasEasy = true;
	MC_LOG_INFO("3ds", "qr: downloading %s\n", url.c_str());
	return true;
}

DsHttpDownload::Status DsHttpDownload::poll(std::string &outError)
{
	outError.clear();
	if (easy == nullptr)
		return Status::Idle;

	int running = 0;
	const CURLMcode mc = curl_multi_perform(multi, &running);
	if (mc != CURLM_OK && mc != CURLM_CALL_MULTI_PERFORM)
	{
		fail("the transfer engine failed (" + std::to_string(static_cast<int>(mc)) + ")",
		     outError);
		return Status::Failed;
	}

	// Progress between slices, for the GUI's bar.
	curl_off_t downloaded = 0;
	curl_off_t contentLength = -1;
	if (curl_easy_getinfo(easy, CURLINFO_SIZE_DOWNLOAD_T, &downloaded) == CURLE_OK)
		received = static_cast<std::uint64_t>(downloaded);
	if (curl_easy_getinfo(easy, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &contentLength) == CURLE_OK &&
	    contentLength > 0)
		total = static_cast<std::uint64_t>(contentLength);

	if (total > static_cast<std::uint64_t>(kMaxDownloadBytes) ||
	    received > static_cast<std::uint64_t>(kMaxDownloadBytes))
	{
		fail("The file is larger than what this console downloads", outError);
		return Status::Failed;
	}

	CURLMsg *msg = nullptr;
	int queued = 0;
	while ((msg = curl_multi_info_read(multi, &queued)) != nullptr)
	{
		if (msg->msg != CURLMSG_DONE || msg->easy_handle != easy)
			continue;

		if (msg->data.result != CURLE_OK)
		{
			fail(describeCurlFailure(msg->data.result), outError);
			return Status::Failed;
		}
		long statusCode = 0;
		curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &statusCode);
		if (statusCode != 200)
		{
			fail("The server answered " + std::to_string(statusCode), outError);
			return Status::Failed;
		}

		std::fflush(file);
		std::fclose(file);
		file = nullptr;
		closeHandles();
		// The file is complete and stays: a later cancel() (the screen
		// closing, the destructor) must not treat it as a partial and
		// delete it.
		destPath.clear();
		MC_LOG_INFO("3ds", "qr: download complete, %llu bytes\n",
		            static_cast<unsigned long long>(received));
		return Status::Done;
	}
	return Status::Busy;
}

void DsHttpDownload::cancel()
{
	if (file != nullptr)
	{
		std::fclose(file);
		file = nullptr;
	}
	closeHandles();
	if (!destPath.empty())
		std::remove(destPath.c_str());
	destPath.clear();
	received = 0;
	total = 0;
}

void DsHttpDownload::closeHandles()
{
	if (multi != nullptr && easy != nullptr && multiHasEasy)
	{
		curl_multi_remove_handle(multi, easy);
		multiHasEasy = false;
	}
	if (easy != nullptr)
	{
		curl_easy_cleanup(easy);
		easy = nullptr;
	}
	if (multi != nullptr)
	{
		curl_multi_cleanup(multi);
		multi = nullptr;
	}
}

void DsHttpDownload::fail(const std::string &reason, std::string &outError)
{
	MC_LOG_WARN("3ds", "qr: download failed: %s\n", reason.c_str());
	outError = reason;
	if (file != nullptr)
	{
		std::fclose(file);
		file = nullptr;
	}
	closeHandles();
	if (!destPath.empty())
		std::remove(destPath.c_str());
	destPath.clear();
}

#endif // CTR_PLATFORM
