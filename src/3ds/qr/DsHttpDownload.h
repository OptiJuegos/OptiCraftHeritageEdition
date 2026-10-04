#pragma once

// DsHttpDownload.h -- a libcurl-backed file download for the "Descarga QR"
// screen (src/3ds/qr/GuiQrDownload.h).
//
// libcurl (3ds-curl, mbedTLS underneath) instead of the system httpc
// service: the console's ssl:C stops at TLS 1.1, and every modern host the
// QR use case actually reaches (Google Drive's uc?export=download flow and
// its drive.usercontent.google.com target, CDNs behind Cloudflare, ...)
// requires TLS 1.2+, so the native stack fails the handshake before a
// single byte moves (rc 0xD8A0A018). curl 8 + mbedTLS also follows
// redirects itself, which the httpc service never did.
//
// The download is driven BY THE GAME LOOP, not a worker thread: the easy
// handle lives inside a curl multi handle and poll() runs one
// curl_multi_perform slice per call, so the progress bar updates at the
// menu's own frame rate and B cancel is just curl_multi_remove_handle --
// no thread join.

#include <cstdint>
#include <cstdio>
#include <string>

typedef void CURL;
typedef void CURLM;

class DsHttpDownload
{
public:
	enum class Status
	{
		Idle,
		Busy,    // making progress; call poll() again
		Done,    // the whole file is at the destination path
		Failed,  // outError holds the reason; the partial file is removed
	};

	DsHttpDownload() = default;
	~DsHttpDownload();

	// Begin a GET of url into destPath. False + a player-readable reason
	// when the transfer cannot even be set up (bad URL, no network stack).
	bool begin(const std::string &url, const std::string &destPath, std::string &outError);

	// One slice of transfer work. Never blocks on purpose, so the menu
	// keeps drawing between calls.
	Status poll(std::string &outError);

	// Abort an in-flight transfer (the whole point of B). Removes the
	// partial file. Safe on a finished or idle download.
	void cancel();

	std::uint64_t receivedBytes() const { return received; }
	std::uint64_t totalBytes() const { return total; }
	bool hasTotalBytes() const { return total != 0; }

	// The filename the final 200 response announced in Content-Disposition
	// ("" when none): a Drive-style URL has no real name in its path.
	const std::string &suggestedFileName() const { return suggestedName; }

private:
	void fail(const std::string &reason, std::string &outError);
	void closeHandles();

	CURL *easy = nullptr;
	CURLM *multi = nullptr;
	bool multiHasEasy = false;
	std::FILE *file = nullptr;
	std::string destPath;
	std::string suggestedName;

	// Filled from curl_easy_getinfo between multi_perform slices.
	std::uint64_t received = 0;
	std::uint64_t total = 0;
};
