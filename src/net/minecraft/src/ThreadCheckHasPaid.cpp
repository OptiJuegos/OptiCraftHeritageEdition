#include "ThreadCheckHasPaid.h"

#include "Minecraft.h"
#include "Session.h"
#include "java/JavaNetwork.h"
#include "java/System.h"

ThreadCheckHasPaid::ThreadCheckHasPaid(Minecraft *minecraft)
	: field_28146_a(minecraft)
{
}

ThreadCheckHasPaid::~ThreadCheckHasPaid()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadCheckHasPaid::start()
{
	worker = std::thread(&ThreadCheckHasPaid::run, this);
	worker.detach();
}

void ThreadCheckHasPaid::run()
{
	if (field_28146_a == nullptr || field_28146_a->session == nullptr)
		return;

	std::string url = "https://login.minecraft.net/session?name="
		+ field_28146_a->session->username
		+ "&session="
		+ field_28146_a->session->sessionId;
	if (JavaNetwork::getResponseCode(url) == 400)
		Minecraft::hasPaidCheckTime = System::currentTimeMillis();
}
