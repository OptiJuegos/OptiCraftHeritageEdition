// GuiQrDownload.cpp -- the "Descarga QR" screen (3DS only).
#ifdef CTR_PLATFORM

#include "3ds/qr/GuiQrDownload.h"

#include <cstdio>
#include <cstring>
#include <vector>

#include "3ds/assets/DsAssetConvert.h"
#include "3ds/qr/DsQrScanner.h"
#include "Minecraft.h"
#include "GuiButton.h"
#include "FontRenderer.h"
#include "Tessellator.h"
#include "TexturePackList.h"
#include "java/File.h"
#include "mods/ModManager.h"
#include "platform/Log.h"
#include "platform/RenderAPI.h"
#include "platform/Storage.h"
#include "skin/SkinManager.h"
#include "GuiMultiplayer.h"
#include "UiStrings.h"
#include "unzip.h"

namespace
{
// The whole download lands here first, is classified and converted, and
// only then moves into skins/, texturepacks/ or mods/.
std::string downloadTempPath()
{
	return PlatformStorage::join("sdmc:/opticraft", "qr-download.tmp");
}

const unsigned char kPngSignature[8] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};

// Touch-sized buttons on the 320x240 panel canvas.
constexpr int kButtonWidth = 130;
constexpr int kButtonHeight = 20;

std::string formatBytes(std::uint64_t bytes)
{
	char buffer[32];
	if (bytes >= 1024 * 1024)
		std::snprintf(buffer, sizeof(buffer), "%llu.%llu MB",
		              static_cast<unsigned long long>(bytes / (1024 * 1024)),
		              static_cast<unsigned long long>((bytes % (1024 * 1024)) / (1024 * 100)));
	else if (bytes >= 1024)
		std::snprintf(buffer, sizeof(buffer), "%llu KB",
		              static_cast<unsigned long long>(bytes / 1024));
	else
		std::snprintf(buffer, sizeof(buffer), "%llu B",
		              static_cast<unsigned long long>(bytes));
	return buffer;
}
} // namespace

GuiQrDownload::GuiQrDownload(GuiScreen *parent)
	: parentScreen(parent)
{
}

GuiQrDownload::~GuiQrDownload()
{
	// onGuiClosed owns the cleanup; this is the last-resort path for a
	// screen that was never shown through displayGuiScreen.
	DsQrScanner::stop();
	download.cancel();
	if (previewTexture >= 0)
	{
		renderDeleteTextures(1, &previewTexture);
		previewTexture = -1;
	}
}

void GuiQrDownload::initGui()
{
	std::string cameraError;
	if (!DsQrScanner::start(cameraError))
	{
		state = State::Failed;
		message = cameraError;
		detail.clear();
		messageIsError = true;
	}
	else if (state == State::Scanning && previewTexture < 0)
	{
		renderGenerateTextures(1, &previewTexture);
	}
	rebuildButtons();
}

void GuiQrDownload::rebuildButtons()
{
	clearControlList();
	switch (state)
	{
	case State::Scanning:
		controlList.push_back(new GuiButton(0, width / 2 - 50, height - 28, 100, kButtonHeight,
		                                    uiText("Cancel")));
		break;
	case State::Confirm:
		controlList.push_back(new GuiButton(1, width / 2 - kButtonWidth - 5, height - 28,
		                                    kButtonWidth, kButtonHeight,
		                                    uiText(scannedKind == Kind::Server ? "Add" : "Download")));
		controlList.push_back(new GuiButton(0, width / 2 + 5, height - 28,
		                                    kButtonWidth, kButtonHeight, uiText("Cancel")));
		break;
	case State::Downloading:
		controlList.push_back(new GuiButton(0, width / 2 - 50, height - 28, 100, kButtonHeight,
		                                    uiText("Cancel")));
		break;
	case State::Installing:
		break; // brief by design; B still cancels out through keyTyped
	case State::Done:
		controlList.push_back(new GuiButton(1, width / 2 - 50, height - 28, 100, kButtonHeight,
		                                    uiText("OK")));
		break;
	case State::Failed:
		controlList.push_back(new GuiButton(0, width / 2 - 50, height - 28, 100, kButtonHeight,
		                                    uiText("Back")));
		break;
	}
}

void GuiQrDownload::beginConfirm(const std::string &url)
{
	scannedUrl = url;
	// A bare host[:port] is a server address, not a download; everything
	// else keeps the download flow, where the real decision is made by the
	// file's content after the bytes arrive.
	if (isServerAddress(url))
	{
		scannedKind = Kind::Server;
	}
	else
	{
		// Extension first, as a hint for the confirm line; the real
		// decision is content-based, after the bytes arrive.
		const std::string lower = String::toLowerCaseJava(jstring(url));
		if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".png") == 0)
			scannedKind = Kind::Skin;
		else if (lower.size() >= 8 && lower.compare(lower.size() - 8, 8, ".ochpack") == 0)
			scannedKind = Kind::Mod;
		else if (lower.size() >= 4 && lower.compare(lower.size() - 4, 4, ".zip") == 0)
			scannedKind = Kind::TexturePack;
		else
			scannedKind = Kind::Unknown;
		installName = nameFromUrl(url);
	}

	// The camera has done its job; give it back before the network takes
	// over so the LED-less sensor is not held through the whole download.
	DsQrScanner::stop();
	state = State::Confirm;
	rebuildButtons();
	MC_LOG_INFO("3ds", "qr: scanned %s (kind %d)\n", url.c_str(), static_cast<int>(scannedKind));
}

bool GuiQrDownload::isServerAddress(const std::string &text)
{
	// host[:port], nothing else: no scheme, no path, no spaces. Meant to
	// catch "play.example.com" / "192.168.1.20:25565" while never matching
	// a URL (the '/' and ':' after the scheme see to that).
	if (text.empty() || text.size() > 255)
		return false;
	const std::size_t colon = text.find(':');
	const std::string host = text.substr(0, colon);
	if (host.empty())
		return false;
	for (char c : host)
	{
		const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
		                (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_';
		if (!ok)
			return false;
	}
	if (colon != std::string::npos)
	{
		const std::string port = text.substr(colon + 1);
		if (port.empty() || port.size() > 5)
			return false;
		for (char c : port)
			if (c < '0' || c > '9')
				return false;
	}
	return true;
}

void GuiQrDownload::addScannedServer()
{
	DsQrScanner::stop();
	std::string error;
	// The address doubles as the list name: scanning a QR gives no room
	// for a friendlier one, and the list shows both fields anyway.
	if (GuiMultiplayer::addServerAndSave(scannedUrl, scannedUrl, error))
	{
		message = "Server added to the multiplayer list";
		detail.clear();
		messageIsError = false;
		state = State::Done;
	}
	else
	{
		message = "The server could not be added";
		detail = error;
		messageIsError = true;
		state = State::Failed;
	}
	rebuildButtons();
}

void GuiQrDownload::startDownload()
{
	downloadPath = downloadTempPath();
	std::string error;
	if (!download.begin(scannedUrl, downloadPath, error))
	{
		state = State::Failed;
		message = error;
		detail = scannedUrl;
		messageIsError = true;
		rebuildButtons();
		return;
	}
	state = State::Downloading;
	rebuildButtons();
}

void GuiQrDownload::updateScreen()
{
	switch (state)
	{
	case State::Scanning:
	{
		std::string url;
		std::string scanError;
		if (DsQrScanner::scan(url, scanError))
			beginConfirm(url);
		else if (!scanError.empty() && !DsQrScanner::isActive())
		{
			// The camera died mid-scan (another app grabbed it, the service
			// went away): say so instead of scanning forever.
			state = State::Failed;
			message = scanError;
			messageIsError = true;
			rebuildButtons();
		}
		break;
	}

	case State::Downloading:
	{
		// Several receive slices per tick: the 20 TPS update would cap a
		// 16 KiB slice at a third of the console's real Wi-Fi rate.
		std::string pollError;
		for (int slice = 0; slice < 4; ++slice)
		{
			const DsHttpDownload::Status status = download.poll(pollError);
			if (status == DsHttpDownload::Status::Busy)
				continue;
			if (status == DsHttpDownload::Status::Done)
			{
				state = State::Installing;
				// The state draws "Installing..." first; the work runs on
				// the NEXT update tick so the player sees it happen.
				installing = false;
				rebuildButtons();
			}
			else if (status == DsHttpDownload::Status::Failed)
			{
				state = State::Failed;
				message = pollError;
				detail = scannedUrl;
				messageIsError = true;
				rebuildButtons();
			}
			break;
		}
		break;
	}

	case State::Installing:
		if (!installing)
		{
			installing = true;
			return; // one tick showing "Installing..." before the blocking work
		}
		runInstall();
		break;

	default:
		break;
	}
}

GuiQrDownload::Kind GuiQrDownload::classifyDownload()
{
	std::FILE *file = std::fopen(downloadTempPath().c_str(), "rb");
	if (file == nullptr)
		return Kind::Unknown;
	unsigned char header[8] = {};
	const std::size_t readCount = std::fread(header, 1, sizeof(header), file);
	std::fclose(file);
	if (readCount >= 8 && std::memcmp(header, kPngSignature, 8) == 0)
		return Kind::Skin;
	if (readCount >= 2 && header[0] == 'P' && header[1] == 'K')
	{
		// A .ochpack is a zip carrying mod.info (or manifest.txt) -- the
		// same probe OchPackReader::readInfoFromBytes uses.
		unzFile archive = unzOpen(downloadTempPath().c_str());
		if (archive == nullptr)
			return Kind::TexturePack;
		const bool isMod = unzLocateFile(archive, "mod.info", 1) == UNZ_OK ||
		                   unzLocateFile(archive, "manifest.txt", 1) == UNZ_OK;
		unzClose(archive);
		return isMod ? Kind::Mod : Kind::TexturePack;
	}
	return Kind::Unknown;
}

void GuiQrDownload::runInstall()
{
	messageIsError = true;
	message.clear();
	detail.clear();

	const Kind kind = classifyDownload();
	// The server may name the file better than the URL did (Drive links
	// land on a Content-Disposition filename); prefer it when present.
	if (!download.suggestedFileName().empty())
		installName = nameFromUrl(download.suggestedFileName());
	if (kind == Kind::Skin)
	{
		// No DsAssetConvert pre-pass for skins: installCustomSkin owns the
		// console's row-order convention at write time (main file and the
		// derived retro/preview PNGs), so a downloaded file goes in
		// untouched -- a convertPng pre-flip would double-flip the pixels it
		// then re-encodes.
		std::string installError;
		if (SkinManager::installCustomSkin(downloadPath, installName, installError))
		{
			message = "Skin installed and selected";
			messageIsError = false;
			state = State::Done;
		}
		else
		{
			message = "The skin could not be installed";
			detail = installError;
			state = State::Failed;
		}
	}
	else if (kind == Kind::TexturePack)
	{
		std::string convertError;
		if (!DsAssetConvert::convertTexturePackZip(downloadPath, convertError))
		{
			message = "Could not convert the texture pack for this console";
			detail = convertError;
			state = State::Failed;
			rebuildButtons();
			return;
		}
		File *dataDir = Minecraft::getMinecraftDir();
		const std::string packsDir = PlatformStorage::join(dataDir->toString(), "texturepacks");
		PlatformStorage::mkdirs(packsDir);
		const std::string finalPath = PlatformStorage::join(packsDir, installName + ".zip");
		std::remove(finalPath.c_str());
		if (std::rename(downloadPath.c_str(), finalPath.c_str()) != 0)
		{
			message = "The texture pack could not be saved";
			detail = packsDir;
			state = State::Failed;
			rebuildButtons();
			return;
		}
		if (mc != nullptr && mc->texturePackList != nullptr)
			mc->texturePackList->updateAvailableTexturePacks();
		message = "Texture pack installed";
		detail = "Select it in Options > Texture Packs";
		messageIsError = false;
		state = State::Done;
	}
	else if (kind == Kind::Mod)
	{
		std::string installError;
		if (ModManager::getInstance().installModPack(downloadPath, installError))
		{
			message = "Mod installed";
			messageIsError = false;
			state = State::Done;
		}
		else
		{
			message = "The mod could not be installed";
			detail = installError;
			state = State::Failed;
		}
	}
	else
	{
		message = "The downloaded file is not a skin, texture pack or mod";
		detail = scannedUrl;
		state = State::Failed;
	}

	// Skins and mods are copied out of the temp by their installers; a pack
	// was renamed into place. Whatever is left here is not needed.
	std::remove(downloadTempPath().c_str());
	rebuildButtons();
}

void GuiQrDownload::keyTyped(char_t c, int_t key)
{
	if (key == 1) // ESC: the menu channel's B
	{
		closeAndReturn();
		return;
	}
	if (key == 28 && state == State::Confirm) // RETURN: the menu channel's A
	{
		if (scannedKind == Kind::Server)
			addScannedServer();
		else
			startDownload();
		return;
	}
	GuiScreen::keyTyped(c, key);
}

void GuiQrDownload::actionPerformed(GuiButton *button)
{
	if (button->id == 0)
	{
		closeAndReturn();
		return;
	}
	if (button->id == 1)
	{
		if (state == State::Confirm)
		{
			if (scannedKind == Kind::Server)
				addScannedServer();
			else
				startDownload();
		}
		else if (state == State::Done || state == State::Failed)
			closeAndReturn();
	}
}

void GuiQrDownload::closeAndReturn()
{
	DsQrScanner::stop();
	download.cancel();
	mc->displayGuiScreen(parentScreen);
}

void GuiQrDownload::onGuiClosed()
{
	// Every exit path releases the camera (a console-wide resource) and
	// aborts an in-flight download. The completed-file case is safe: the
	// downloader forgets the destination path once it reports Done.
	DsQrScanner::stop();
	download.cancel();
	if (previewTexture >= 0)
	{
		renderDeleteTextures(1, &previewTexture);
		previewTexture = -1;
	}
	GuiScreen::onGuiClosed();
}

void GuiQrDownload::drawPreview(int_t x, int_t y, int_t w, int_t h)
{
	if (previewTexture < 0 || !DsQrScanner::isActive())
		return;
	renderBindTexture(previewTexture);
	if (renderTextureBeginUpload(previewTexture,
	                             static_cast<int>(DsQrScanner::previewWidth()),
	                             static_cast<int>(DsQrScanner::previewHeight()),
	                             0, false, false))
	{
		renderTextureImageRgba(0, static_cast<int>(DsQrScanner::previewWidth()),
		                       static_cast<int>(DsQrScanner::previewHeight()),
		                       DsQrScanner::previewFrame());
	}

	renderEnable(RenderCapability::Texture2D);
	renderEnable(RenderCapability::Blend);
	renderBlendFunc(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
	renderColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	Tessellator &tess = Tessellator::instance;
	tess.startDrawingQuads();
	tess.addVertexWithUV(x, y + h, zLevel, 0.0, 1.0);
	tess.addVertexWithUV(x + w, y + h, zLevel, 1.0, 1.0);
	tess.addVertexWithUV(x + w, y, zLevel, 1.0, 0.0);
	tess.addVertexWithUV(x, y, zLevel, 0.0, 0.0);
	tess.draw();
}

void GuiQrDownload::drawProgressBar(int_t x, int_t y, int_t w, int_t h)
{
	drawRect(x - 1, y - 1, x + w + 1, y + h + 1, 0xFF808080);
	drawRect(x, y, x + w, y + h, 0xFF202020);
	std::uint64_t done = 0;
	std::uint64_t total = 0;
	if (download.hasTotalBytes() && download.totalBytes() != 0)
	{
		total = download.totalBytes();
		done = download.receivedBytes();
		if (done > total)
			done = total;
		const int_t fill = static_cast<int_t>((done * static_cast<std::uint64_t>(w)) / total);
		drawRect(x, y, x + fill, y + h, 0xFF55FF55);
	}
	else
	{
		// Unknown length: a moving eighth-window so the bar still shows life.
		total = 0;
		done = download.receivedBytes();
		const int_t phase = static_cast<int_t>((done / (32 * 1024)) % 8);
		const int_t fill = w / 8;
		const int_t origin = x + phase * fill;
		const int_t clippedFill = std::min(fill, static_cast<int_t>(x + w - origin));
		if (clippedFill > 0)
			drawRect(origin, y, origin + clippedFill, y + h, 0xFF55FF55);
	}
}

std::string GuiQrDownload::kindLabel(Kind kind)
{
	switch (kind)
	{
	case Kind::Skin: return "Skin";
	case Kind::TexturePack: return "Texture pack";
	case Kind::Mod: return "Mod";
	case Kind::Server: return "Minecraft server";
	default: return "Unknown file";
	}
}

std::string GuiQrDownload::nameFromUrl(const std::string &url)
{
	std::string path = url;
	const std::size_t cut = path.find_first_of("?#");
	if (cut != std::string::npos)
		path = path.substr(0, cut);
	const std::size_t slash = path.find_last_of('/');
	std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
	const std::size_t dot = name.find_last_of('.');
	if (dot != std::string::npos && dot > 0)
		name = name.substr(0, dot);

	std::string safe;
	for (char c : name)
	{
		if (safe.size() >= 32)
			break;
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
		    c == '_' || c == '-' || c == ' ')
			safe += c;
	}
	if (safe.empty())
		safe = "QRDownload";
	return safe;
}

void GuiQrDownload::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	const int_t centreX = width / 2;

	drawCenteredString(fontRenderer, uiText("QR Download"), centreX, 8, 0xFFFFFF);

	switch (state)
	{
	case State::Scanning:
	{
		const int_t previewW = 200;
		const int_t previewH = 150;
		drawPreview(centreX - previewW / 2, 30, previewW, previewH);
		if (DsQrScanner::hasReceivedFrame())
			drawCenteredString(fontRenderer, "Point the back camera at a QR code",
			                   centreX, height - 58, 0xAAAAAA);
		else
			drawCenteredString(fontRenderer, "Waiting for camera frames...",
			                   centreX, height - 58, 0xFF5555);
		break;
	}

	case State::Confirm:
	{
		drawCenteredString(fontRenderer, kindLabel(scannedKind), centreX, 28, 0xFFFF55);
		// Wrap the URL over up to three lines; QR URLs are short but a
		// shortener link is not always.
		std::string rest = scannedUrl;
		for (int line = 0; line < 3 && !rest.empty(); ++line)
		{
			std::string chunk = rest;
			if (chunk.size() > 48)
				chunk = chunk.substr(0, 48);
			rest = rest.size() > chunk.size() ? rest.substr(chunk.size()) : std::string();
			drawCenteredString(fontRenderer, chunk, centreX, 44 + line * 12, 0xCCCCCC);
		}
		drawCenteredString(fontRenderer,
		                   scannedKind == Kind::Server ? "(A) Add    (B) Cancel"
		                                               : "(A) Download    (B) Cancel",
		                   centreX, height - 52, 0xAAAAAA);
		break;
	}

	case State::Downloading:
	{
		drawCenteredString(fontRenderer, kindLabel(scannedKind), centreX, 34, 0xFFFF55);
		drawProgressBar(centreX - 140, 60, 280, 10);
		if (download.hasTotalBytes() && download.totalBytes() != 0)
			drawCenteredString(fontRenderer,
			                   formatBytes(download.receivedBytes()) + " / " +
			                       formatBytes(download.totalBytes()),
			                   centreX, 78, 0xCCCCCC);
		else
			drawCenteredString(fontRenderer, formatBytes(download.receivedBytes()) + " received",
			                   centreX, 78, 0xCCCCCC);
		break;
	}

	case State::Installing:
		drawCenteredString(fontRenderer, "Installing...", centreX, 48, 0xFFFFFF);
		drawCenteredString(fontRenderer, "(a few seconds)", centreX, 64, 0x888888);
		break;

	case State::Done:
	case State::Failed:
	{
		const int colour = messageIsError ? 0xFF5555 : 0x55FF55;
		drawCenteredString(fontRenderer, message, centreX, 44, colour);
		if (!detail.empty())
			drawCenteredString(fontRenderer, detail, centreX, 60, 0x888888);
		break;
	}
	}

	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

#endif // CTR_PLATFORM
