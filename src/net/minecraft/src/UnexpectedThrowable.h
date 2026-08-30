#pragma once

#include <stdexcept>
#include "java/String.h"

// net.minecraft.src.UnexpectedThrowable
class UnexpectedThrowable
{
public:
	UnexpectedThrowable(const jstring &s, std::exception_ptr throwable = std::current_exception())
		: description(s), exception(throwable) {}

	const jstring description;
	const std::exception_ptr exception;
};
