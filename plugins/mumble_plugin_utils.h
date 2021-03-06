// Copyright 2005-2020 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MUMBLE_PLUGIN_UTILS_H_
#define MUMBLE_MUMBLE_PLUGIN_UTILS_H_

#include <codecvt>
#include <locale>

#ifdef OS_LINUX
#	include <fenv.h>
#endif

#ifdef _MSC_VER
#	include <intrin.h>
#endif

/// This union is used by isBigEndian() to determine the endianness.
union SingleSplit4Bytes {
	uint32_t single;
	uint8_t split[4];

	constexpr SingleSplit4Bytes(const uint32_t value) : single(value) {}
};

static inline std::string utf16ToUtf8(const std::wstring &wstr) {
	std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > conv;
	return conv.to_bytes(wstr);
}

// escape lossily converts the given
// string to ASCII, replacing any
// character not within the printable
// ASCII region (32-126) with an ASCII
// space character.
//
// escape also replaces any double quote
// characters with an ASCII space. This
// allows the string to be safely used
// when constructing JSON documents via
// string concatenation.
//
// Finally, escape ensures that the given
// string is NUL-terminated by always
// setting the last byte of the input
// string to the value 0.
static inline void escape(char *str, const size_t &size) {
	// Ensure the input string is properly NUL-terminated.
	str[size - 1] = 0;
	char *c       = str;

	while (*c != '\0') {
		// For JSON compatibility, the string
		// can't contain double quotes.
		// If a double quote is found, replace
		// it with an ASCII space.
		if (*c == '"') {
			*c = ' ';
		}

		// Ensure the string is within printable
		// ASCII. If not, replace the offending
		// byte with an ASCII space.
		if (*c < 32 || *c > 126) {
			*c = ' ';
		}

		c += 1;
	}
}

/// Calculates sine and cosine of the specified value.
/// On Linux the calculation is guaranteed to be simultaneous.
static inline bool sinCos(const float value, float &outSin, float &outCos) {
#ifdef OS_WINDOWS
	outSin = sin(value);
	outCos = cos(value);
	return true;
#else
	errno = 0;
	feclearexcept(FE_ALL_EXCEPT);

	sincosf(value, &outSin, &outCos);

	return fetestexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW) == 0;
#endif
}

/// Converts degrees to radians.
static inline float degreesToRadians(const float degrees) {
	constexpr float piOver180 = M_PI / 180.0f;
	return piOver180 * degrees;
}

/// Detects whether the architecture is big-endian.
static constexpr bool isBigEndian() {
	// An union allows access to a single byte without the need for a cast.
	return SingleSplit4Bytes(0x01020304).split[0] == 1;
}

/// Converts from network byte order to host byte order.
static inline uint16_t networkToHost(const uint16_t value) {
	if (isBigEndian()) {
		return value;
	}
#ifdef _MSC_VER
	return _byteswap_ushort(value);
#else
	return __builtin_bswap16(value);
#endif
}

#endif
