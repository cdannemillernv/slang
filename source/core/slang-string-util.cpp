#include "slang-string-util.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangBlob = SLANG_UUID_ISlangBlob;

/* static */ISlangUnknown* StringBlob::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangBlob) ? static_cast<ISlangBlob*>(this) : nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void StringUtil::split(const UnownedStringSlice& in, char splitChar, List<UnownedStringSlice>& slicesOut)
{
    slicesOut.clear();

    const char* start = in.begin();
    const char* end = in.end();

    while (start < end)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        // Add to output
        slicesOut.add(UnownedStringSlice(start, cur));

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }
}

/* static */int StringUtil::indexOfInSplit(const UnownedStringSlice& in, char splitChar, const UnownedStringSlice& find)
{
    const char* start = in.begin();
    const char* end = in.end();

    for (int i = 0; start < end; ++i)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        // See if we have a match
        if (UnownedStringSlice(start, cur) == find)
        {
            return i;
        }

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }
    return -1;
}

UnownedStringSlice StringUtil::getAtInSplit(const UnownedStringSlice& in, char splitChar, int index)
{
    const char* start = in.begin();
    const char* end = in.end();

    for (int i = 0; start < end; ++i)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        if (i == index)
        {
            return UnownedStringSlice(start, cur);
        }

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }

    return UnownedStringSlice();
}

/* static */size_t StringUtil::calcFormattedSize(const char* format, va_list args)
{
#if SLANG_WINDOWS_FAMILY
    return _vscprintf(format, args);
#else
     return vsnprintf(nullptr, 0, format, args);
#endif
}

/* static */void StringUtil::calcFormatted(const char* format, va_list args, size_t numChars, char* dst)
{
#if SLANG_WINDOWS_FAMILY
    vsnprintf_s(dst, numChars + 1, _TRUNCATE, format, args);
#else
    vsnprintf(dst, numChars + 1, format, args);
#endif
}

/* static */void StringUtil::append(const char* format, va_list args, StringBuilder& buf)
{
    // Calculate the size required (not including terminating 0)
    size_t numChars;
    {
        // Create a copy of args, as will be consumed by calcFormattedSize
        va_list argsCopy;
        va_copy(argsCopy, args);
        numChars = calcFormattedSize(format, argsCopy);
        va_end(argsCopy);
    }

    // Requires + 1 , because calcFormatted appends a terminating 0
    char* dst = buf.prepareForAppend(numChars + 1);
    calcFormatted(format, args, numChars, dst);
    buf.appendInPlace(dst, numChars);
}

/* static */void StringUtil::appendFormat(StringBuilder& buf, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    append(format, args, buf);
    va_end(args);
}

/* static */String StringUtil::makeStringWithFormat(const char* format, ...)
{
    StringBuilder builder;

    va_list args;
    va_start(args, format);
    append(format, args, builder);
    va_end(args);

    return builder;
}

/* static */String StringUtil::getString(ISlangBlob* blob)
{
    if (blob)
    {
        size_t size = blob->getBufferSize();
        if (size > 0)
        {
            const char* contents = (const char*)blob->getBufferPointer();
            // Check it has terminating 0, if not we must construct as if it does
            if (contents[size - 1] == 0)
            {
                size--;
            }
            return String(contents, contents + size);
        }
    }
    return String();
}

ComPtr<ISlangBlob> StringUtil::createStringBlob(const String& string)
{
    return ComPtr<ISlangBlob>(new StringBlob(string));
}

/* static */String StringUtil::calcCharReplaced(const UnownedStringSlice& slice, char fromChar, char toChar)
{
    if (fromChar == toChar)
    {
        return slice;
    }

    const Index numChars = slice.size();
    const char* srcChars = slice.begin();

    StringBuilder builder;
    char* dstChars = builder.prepareForAppend(numChars);

    for (Index i = 0; i < numChars; ++i)
    {
        char c = srcChars[i];
        dstChars[i] = (c == fromChar) ? toChar : c;
    }

    builder.appendInPlace(dstChars, numChars);
    return builder;
}

/* static */String StringUtil::calcCharReplaced(const String& string, char fromChar, char toChar)
{
    return (fromChar == toChar || string.indexOf(fromChar) == Index(-1)) ? string : calcCharReplaced(string.getUnownedSlice(), fromChar, toChar);
}



} // namespace Slang
