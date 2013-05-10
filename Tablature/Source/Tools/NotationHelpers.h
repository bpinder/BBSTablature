#ifndef NOTATION_HELPERS_H
#define NOTATION_HELPERS_H

namespace belle
{
    using namespace bellebonnesage;
}

namespace NotationHelper
{
    static inline double pixelsToInches (const int pixels, const int pixelsPerInch) noexcept
    {
        return ((double)pixels / (double)pixelsPerInch);
    }

    static inline double inchesToPixels (const double inches, const int pixelsPerInch) noexcept
    {
        return (inches * pixelsPerInch);
    }

    static inline double inchesToSpaces (const double inches, const double spaceHeightInInches) noexcept
    {
        return (inches / spaceHeightInInches);
    }

    static inline double spacesToInches (const double spaces, const double spaceHeightInInches) noexcept
    {
        return (spaces * spaceHeightInInches);
    }

    static inline double pixelsToSpaces (const int pixels, const int pixelsPerInch, const double spaceHeightInInches) noexcept
    {
        return (inchesToSpaces (pixelsToInches (pixels, pixelsPerInch), spaceHeightInInches));
    }

    static inline double spacesToPixels (const double spaces, const double spaceHeightInInches, const int pixelsPerInch) noexcept
    {
        return (inchesToPixels (spacesToInches (spaces, spaceHeightInInches), pixelsPerInch));
    }
}

#endif // NOTATION_HELPERS_H