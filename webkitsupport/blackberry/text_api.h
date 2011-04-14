/**
TEXT_API.H
Copyright (C) 2009-2010 Research In Motion Ltd.

Started by Graham Asher, 17th November 2009.

A dependency-free general API for text-related functions.
This includes drawing and measuring text, finding breaks, and finding
character properties.
*/

#ifndef TEXT_API_H_INCLUDED__
#define TEXT_API_H_INCLUDED__

/*
Macros to export public functions from a DLL.
If you need them, define TEXT_API_NEEDS_DLL_EXPORTS.
They will be defined if TEXT_API_IMPORT is not already defined.
*/

#ifndef TEXT_API_IMPORT
    
    #if defined(_MSC_VER)
        #define TEXT_API_IMPORT __declspec(dllexport)
        #define TEXT_API_EXPORT __declspec(dllexport)

    #elif defined(__GCC32__)
        #define TEXT_API_IMPORT
        #define TEXT_API_EXPORT __declspec(dllexport)
    #endif

#endif


#ifndef TEXT_API_NEEDS_DLL_EXPORTS
    #undef TEXT_API_IMPORT
    #undef TEXT_API_EXPORT
    #define TEXT_API_IMPORT
    #define TEXT_API_EXPORT
#endif


namespace TextAPI
    {
    
    /** The maximum length of a font family name, including the trailing null. */
    const int MAX_FONT_NAME = 32;

    /** A 32-bit color with 8 bits each for red, green, blue and alpha. */ 
    class Color
        {
        public:
        Color(): m_value(0) { }
        Color(unsigned int value): m_value(value) { }
        operator unsigned int() const { return m_value; }
        void set(unsigned int red,unsigned int green,unsigned int blue,unsigned int alpha)
            {
            m_value = (unsigned int)((((unsigned int)((red) & 0xFF)) << RED_SHIFT) |
                                     (((unsigned int)((green) & 0xFF)) << GREEN_SHIFT) |
                                     (((unsigned int)((blue) & 0xFF)) << BLUE_SHIFT) |
                                     (((unsigned int)((alpha) & 0xFF)) << ALPHA_SHIFT));
            }

        /** Create a color from integer values in the range 0...255. */
        Color(unsigned int red,unsigned int green,unsigned int blue,unsigned int alpha)
            {
            set(red,green,blue,alpha);
            }

        /** Create a color from floating-point values. Values are clamped to the range 0...1. */
        Color(double red,double green,double blue,double alpha)
            {
            int r = (int)(red * 255 + 0.5); if (r < 0) r = 0; else if (r > 255) r = 255;
            int g = (int)(green * 255 + 0.5); if (g < 0) g = 0; else if (g > 255) g = 255;
            int b = (int)(blue * 255 + 0.5); if (b < 0) b = 0; else if (b > 255) b = 255;
            int a = (int)(alpha * 255 + 0.5); if (a < 0) a = 0; else if (a > 255) a = 255;
            set(r,g,b,a);
            }

        unsigned int getRed() const { return (m_value & RED_MASK) >> RED_SHIFT; }
        unsigned int getGreen() const { return (m_value & GREEN_MASK) >> GREEN_SHIFT; }
        unsigned int getBlue() const { return (m_value & BLUE_MASK) >> BLUE_SHIFT; }
        unsigned int getAlpha() const { return (m_value & ALPHA_MASK) >> ALPHA_SHIFT; }

        enum
            {
            RED_MASK = 0xFF000000,
            RED_SHIFT = 24,
            GREEN_MASK = 0xFF0000,
            GREEN_SHIFT = 16,
            BLUE_MASK = 0xFF00,
            BLUE_SHIFT = 8,
            ALPHA_MASK = 0xFF,
            ALPHA_SHIFT = 0
            };

        unsigned int m_value;
        };

    /** Return codes are 0 for success, non-zero otherwise. */
    typedef int ReturnCode;

    /** For now, define a 16-bit character type here to preserve the no-dependency rule. */
    typedef unsigned short Utf16Char;

    /** Units for dimensions. */
    enum Unit
        {
        /** The units in which the font height is specified. User units are transformed to pixels by the font transform. */
        UserUnit,
        /** An em is equal to the font height. */
        EmUnit
        };

    /**
    Dimensions can be specified either relatively in em or absolutely in user units.
    User units are transformed to pixels by the font transform.
    */
    class Dimension
        {
        public:
        Dimension():
            m_value(0),
            m_unit(UserUnit)
            {
            }

        /** The value. */
        double m_value;
        /** The unit in which the value is expressed. */
        Unit m_unit;        
        };
    
    /**
    Colors that may be inherited or specified explicitly.
    */ 
    class ColorSpec
        {
        public:
        ColorSpec():
            m_color(0),
            m_useColor(false)
            {
            }

        /** The color. */
        Color m_color;
        /** If true, use m_color; if false, inherit the color. */
        bool m_useColor;
        };

    /** Font styles. */
    enum Style
        {
        /** The default style; this value can be used for 'don't care'. */
        NoStyle,
        /** A normal, upright style. */
        PlainStyle,
        /** Use the italic variant for this family. */
        ItalicStyle,
        /** Use the oblique (slanted) variant for this family. */
        ObliqueStyle
        };

    /** Font variants. */
    enum Variant
        {
        /** The default variant; this value can be used for 'don't care'. */
        NoVariant,
        /** A plain font. */
        PlainVariant,
        /** A small-caps font. */
        SmallCapsVariant
        };

    /** Shadow types. */
    enum ShadowType
        {
        /** No shadow. */
        NoShadow,
        /** Draw a blurred shadow. */
        BlurredShadow,
        };

    /** A shadow specification. */
    class Shadow
        {
        public:
        Shadow():
            m_type(NoShadow)
            {
            }

        /** The type of shadow. */
        ShadowType m_type;
        /** The x offset of the shadow. */
        Dimension m_xOffset;
        /** The y offset of the shadow; y coordinates increase downwards. */
        Dimension m_yOffset;
        /** The radius of the blur. */
        Dimension m_blurRadius;
        /** The color of the glyph; the default (inherited) color is null. */
        ColorSpec m_glyphColor;
        /** The color of the shadow; the default (inherited) color is the current text color. */
        ColorSpec m_shadowColor;
        };

    /** Outline effect types. */
    enum OutlineEffectType
        {
        /** Do not apply the outline effect. */
        NoOutlineEffect,
        /** Apply the standard outline effect. */
        StandardOutlineEffect
        };

    /** An outline effect specification. */
    class OutlineEffect
        {
        public:
        OutlineEffect():
            m_type(NoOutlineEffect)
            {
            m_penWidth.m_value = 1.0 / 32.0;
            m_penWidth.m_unit = EmUnit;
            }

        /** The type of outline. */        
        OutlineEffectType m_type;
        /** The pen width used to stroke the outline. */
        Dimension m_penWidth;
        /** The color of the glyph; the default (inherited) color is null. */
        ColorSpec m_glyphColor;
        /** The outline color; the default (inherited) color is the current text color. */
        ColorSpec m_outlineColor;
        };

    /**
    Enumerated constants for types of underlining, strikethrough and overline.
    Underlining is specified by the bits 0-7, strikethrough by the bits
    8-15, and overlining by bits 16-23. Underlining, strikethrough and
    overlining can be combined by combining the three types of value.
    */
    enum Underline
	    {
	    NoUnderline = 0,
	    StandardUnderline = 1,
	    BrokenUnderline = 2,
	    DottedUnderline = 3,
	    WavyUnderline = 4,
	    NoStrikethrough = 0,
	    StandardStrikethrough = 1 << 8,
	    BrokenStrikethrough = 2 << 8,
	    DottedStrikethrough = 3 << 8,
	    WavyStrikethrough = 4 << 8,
	    NoOverline = 0,
	    StandardOverline = 1 << 16,
	    BrokenOverline = 2 << 16,
	    DottedOverline = 3 << 16,
	    WavyOverline = 4 << 16,

	    UnderlineMask = 0xFF,
	    StrikethroughMask = 0xFF00,
	    OverlineMask = 0xFF0000
	    };

    /** Enumerated constants for glyph hinting. */
    enum HintingMode
        {
        /** The best hinting method for the combination of font and font technology in use. */
        DefaultHinting,
        /** No hinting. */
        NoHinting,
        /** A light auto-hinting method to avoid 'dirt' in anti-aliased glyphs. */
        LightAutoHinting,
        /** Standard auto-hinting. */ 
        StandardAutoHinting,
        /** Standard hinting using the font's own data if available. */
        StandardHinting
        };

    /** A specification for a named font at a particular height and weight. */
    class FontSpec
        {
        public:
        FontSpec():
            m_height(0),
            m_weight(0),
            m_style(NoStyle),
            m_variant(NoVariant),
            m_monospace(false),
            m_underline(NoUnderline),
            m_hintingMode(DefaultHinting)
            {
            for (int i = 0; i < MAX_FONT_NAME; i++)
                m_name[i] = 0;
            m_transform[0] = m_transform[3] = 1;
            m_transform[1] = m_transform[2] = 0;
            }

        /**
        The family name as a null-terminated UTF16 string of maximum MAX_FONT_NAME bytes
        including the terminating null. An empty string means 'don't care' (use a default font).
        */
        Utf16Char m_name[MAX_FONT_NAME];
        /** Height in user units. The value 0 means 'don't care' (use a default size). */
        double m_height;
        /** Weight, defined using CSS numeric values: http://www.w3.org/TR/CSS2/fonts.html#font-boldness. The value 0 means 'don't care' or 'default'. */
        int m_weight;
        /** The style: plain, italic, etc. */
        Style m_style;
        /** The variant: normal, small-caps, etc. */
        Variant m_variant;
        /** The monospace type: true if monospaced. */
        bool m_monospace;
        /**
        A partial 2D affine transform, with no translation, to be applied to all characters.
        Any rotation affects the baseline angle. The transform may be used for algorithmically
        expanded or condensed fonts without affecting the baseline angle.
        */
        double m_transform[4];
        /** The outline effect if any. */
        OutlineEffect m_outline;
        /** The shadow if any. */
        Shadow m_shadow;
        /** The underline, strikethrough and overline if any. */
        Underline m_underline;
        /** Hinting mode. */
        HintingMode m_hintingMode;
        };

    /** Word space modes. */
    enum WordSpaceMode
        {
	    /** Use the natural word spacing. */           
	    WordSpaceNatural,
	    /** The supplied word space size is used instead of the natural word space size. */
	    WordSpaceOverride,
	    /** The supplied word space size is added to the natural word space size. */
	    WordSpaceAdd,
	    /** The supplied word space size is multiplied by the natural word space size. */
	    WordSpaceMultiply
	    };

    /** A word space specification. */
    class WordSpace
        {
        public:
        WordSpace():
            m_mode(WordSpaceNatural)
            {
            }

        /** The mode: that is, the interpretation of _size. */
        WordSpaceMode m_mode;
        /** The size of the word space. */
        Dimension m_size;
        };

    /** Text case transformation. */
    enum CaseTransform
        {
	    /** Do not change the case of the text. */
	    NoCaseTransform,
	    /** Transform to lower case. */
	    LowerCaseTransform,
	    /** Transform to title case (applies to the first letter of each word when changing case; other letters are changed to lower-case). */
	    TitleCaseTransform,
	    /** Transform to upper case. */
	    UpperCaseTransform,
	    /** Capitalize (for CSS compatibility: change the first letter of each word to title case but don't change other letters). */
	    CapitalizeCaseTransform
	    };

	/**
	An enumerated type to state whether the text is to be reversed or has already been reversed,
	or requires bidirectional reordering.
	*/
	enum TextOrder
		{
		/**
		Draw text without reversing or reordering it in any way.
		This setting is used for left-to-right runs where bidirectional
		reordering has already been done.
		*/
		NoTextOrder,			
		/**
		Reverse the text before drawing.
		This setting is used for right-to-left runs where bidirectional
		reordering has already been done, and the text is known to be
        right-to-left, but has not yet been reversed.
        
        When text is reversed, combining characters are reordered so that
        they remain after their base characters and the combinations are
        drawn correctly.
		*/
		ReverseTextOrder,
		/**
		Draw text without reversing.
		This setting is used for right-to-left runs where bidirectional
		reordering has already been done and the text has already been reversed.
		*/
		AlreadyReversedTextOrder,
		/**
		Perform bidirectional reordering on the text
		then draw it as a series of left-to-right and right-to-left runs.
		*/
		BidiTextOrder
		};

    /** Parameters used to modify the way the text is drawn: glyph effects, spacing, etc. */
    class DrawParam
        {
        public:
        DrawParam():
            m_caseTransform(NoCaseTransform),
            m_textOrder(BidiTextOrder)
            {
            }

        /** Space to be added between each letter and the next. */ 
        Dimension m_letterspace;
        /** Control of spacing between words. */
        WordSpace m_wordspace;
        /** Case transformation. */
        CaseTransform m_caseTransform;
        /** Text order. */
        TextOrder m_textOrder;
        };

    /** Metrics returned by a text drawing or measuring operation. */
    class TextMetrics
        {
        public:
        TextMetrics():
            m_consumed(0),
            m_newX(0),
            m_newY(0),
            m_linearAdvance(0),
            m_boundsLeft(0),
            m_boundsTop(0),
            m_boundsRight(0),
            m_boundsBottom(0),
            m_ascent(0),
            m_descent(0)
            {
            }

        /**
        The number of UTF16 code units (16-bit words) of text consumed,
        which will be less than the length of the text if the wrap width is reached first.
        */
        int m_consumed;
        /** The x coordinate of the drawing position after the text has been drawn. */
        double m_newX;
        /** The y coordinate of the drawing position after the text has been drawn. */
        double m_newY;
        /** The linear advance, measured along the baseline. */
        double m_linearAdvance;
        /** The left edge of the bounds of the inked area. */
        double m_boundsLeft;
        /** The top edge of the bounds of the inked area. */
        double m_boundsTop;
        /** The right edge of the bounds of the inked area. */
        double m_boundsRight;
        /** The bottom edge of the bounds of the inked area. */
        double m_boundsBottom;
        /**
	    The	ideal ascent for the text. This is the maximum of the ideal ascents of any fonts used by the text.
	    The ascent includes both the ordinary ascent and any leading above.
	    */
        double m_ascent;
	    /**
	    The	ideal descent for the text. This is the maximum of the ideal descents ascents of any fonts used by the text.
	    The descent includes both the ordinary descent and any leading below.
	    */
        double m_descent;
        };

    /** Handles are used to hide implementations. */
    class Handle
        {
        public:
        virtual ~Handle() { }
        };

    /** Graphics context types. */
    enum GraphicsContextType
        {
        /** A bitmap graphics context, primarily for testing. */
        BitmapGraphicsContext,
        /** An OpenVG graphics context. */
        OpenVGGraphicsContext
        };

    /** An opaque type for the native graphics display: for example, EGLDisplay. */
    typedef void* NativeGraphicsDisplay;

    /** An opaque type for the native graphics context: for example, EGLContext. */
    typedef void* NativeGraphicsContext;

    /** An opaque type for the native graphics surface: for example, EGLSurface. */
    typedef void* NativeGraphicsSurface;

    /** A graphics context provides a surface that can be drawn to. */
    class GraphicsContext: public Handle
        {
        friend class Engine;
        
        public:
        /** Set the native display (e.g., the EGL display). Set the context and surface to null. */
        virtual ReturnCode setDisplay(NativeGraphicsDisplay) { return 0; }
        /**
        Set the native context (e.g., the EGL context).
        For the OpenVG graphics context, this function 
        sets the EGL context and synchronises the current graphics parameters to it, including
        the fill and stroke paint. That allows you to draw text using the current OpenVG
        paint without explicitly setting the paint using setFillColor and setStrokeColor.

        If you later change the paint using OpenVG, the TextAPI::GraphicsContext object will become
        out of synch with OpenVG (normally a GC needs to know what its current parameters are);
        you can either (i) ignore that situation, which is fine as long as you never call
        setFillColor or setStrokeColor, and never set explicit colors for text effects like blurring;
        or (ii) you can explicitly re-synch the GC by calling TextAPI::GraphicsContext::setContext again.
        */
        virtual ReturnCode setContext(NativeGraphicsContext) { return 0; }
        /** Set the native surface (e.g., the EGL surface). */
        virtual ReturnCode setSurface(NativeGraphicsSurface) { return 0; }
        /** Return the type of the graphics context. */
        virtual GraphicsContextType getType() = 0;
        /** Return the native graphics surface if possible. Mainly used for debugging*/
        virtual NativeGraphicsSurface getSurface() { return 0; }
        /** Set the fill color. For OpenVG graphics contexts, see the note on setContext. */
        virtual void setFillColor(Color aColor) = 0;
        /** Set the stroke color. For OpenVG graphics contexts, see the note on setContext. */
        virtual void setStrokeColor(Color aColor) = 0;
        };

    /**
    A font data ID is used to identify font data (i.e., a typeface) so that
    it can be unloaded at some time after it has been loaded.
    */
    typedef unsigned int FontDataId;

    /**
    Metrics for a particular instance (size etc.) of a font.
    All sizes are in user units.
    */
    class FontMetrics
        {
        public:
        FontMetrics():
            m_height(0),
            m_ascent(0),
            m_maxAscent(0),
            m_descent(0),
            m_maxDescent(0),
            m_underlineOffset(0),
            m_underlineWeight(0),
            m_leadingAbove(0),
            m_leadingBelow(0),
            m_spaceWidth(0),
            m_maxCharWidth(0),
            m_averageCharWidth(0)
            {
            }

	    /** Height of the font in units per em. */
	    double m_height;
	    /** Height of standard ascenders (Latin capital letters). */
	    double m_ascent;
	    /** The maximum ascent of any character. */
	    double m_maxAscent;
	    /** Depth of standard descenders (usually of the Latin letters p, q, y). */
	    double m_descent;
	    /** The maximum descent of any character. */
	    double m_maxDescent;
	    /** Distance of underline below baseline. */
	    double m_underlineOffset;
	    /** Standard underline weight. */
	    double m_underlineWeight;
	    /** Standard leading above the line. */
	    double m_leadingAbove;
	    /** Standard leading below the line. */
	    double m_leadingBelow;
	    /** The width of a word space. */
	    double m_spaceWidth;
	    /** The maximum width of a character. */
	    double m_maxCharWidth;
	    /** The average width of a character for the purpose of calculating text field sizes. */
	    double m_averageCharWidth;
        };
    
    /**
    A font is a handle to an underlying object that accesses actual typefaces and supplies glyphs.
    Fonts are created by the Engine.
    */
    class Font: public Handle
        {
        public:
        /** Set the font from the parameters in a font spec. */
        virtual ReturnCode setFontSpec(const FontSpec& fontspec) = 0;
        /** Return the normalized font spec; not necessarily exactly the same as the font spec passed to setFontSpec. */
        virtual void getFontSpec(FontSpec& fontspec) = 0;
        /** Return the font spec after font matching. */
        virtual void getMatchedFontSpec(FontSpec& fontspec) = 0;
        /** Set the font height in user units per em. */
        virtual ReturnCode setHeight(double height) = 0;
        /** Compare a font for equality with another font. */
        virtual bool operator==(const Font& font) = 0;
        /** Return the font metrics. */
        virtual void getFontMetrics(FontMetrics& metrics) = 0; 
        };

    /** Data and function pointers allowing font data to be read. */
    class Stream
        {
        public:
        /** Arbitrary user data, such as a file handle. */
        void* m_data;
        /** A function to read count bytes, starting at offset, into buffer. */
        int (*m_read)(class Stream& stream,int offset,unsigned char* buffer,int count);
        /** A function to close the stream. */
        void (*m_close)(class Stream& stream);
        /** A function to return the length of the stream in bytes. */
        int (*m_getLength)(class Stream& stream);
        };

    enum GlyphType
        {
        /** Fonts create bitmap glyphs. */
        BitmapGlyphType,
        /** Fonts create glyphs that use OpenVG paths if possible. */
        OpenVGGlyphType
        };

	/**
	How to round the character position returned when converting an x position
	into a text position.
	*/
	enum TextPosRounding
		{
		/** Round to the position left of the point. */
		TextPosRoundLeft,
		/** Round to the position nearest the point. */
		TextPosRoundNearest,
		/** Round to the position right of the point. */
		TextPosRoundRight,
		/**
		Round to the position logically preceding the point.
		Use this mode to get the index of the character under the point.
		*/
		TextPosRoundPreceding,
		/** Round to the position logically following the point. */
		TextPosRoundFollowing
		};

    /**
    Optional advanced font loading parameters.
    This class exists to make it easy to add further advanced parameters
    (e.g., overriding style) when needed without changing the signature
    of all the loadFont functions.
    */
    class AdvancedFontLoadingParam
        {
        public:
        AdvancedFontLoadingParam():
            m_privateEncoding(false),
            m_defaultHintingMode(DefaultHinting)
            {
            }

        /**
        If m_privateEncoding is true mark the typeface as having a private encoding
        so that it will not be used as a fallback typeface if a character cannot be found.
        This setting is useful for wingdings and other special symbol fonts.
        */
        bool m_privateEncoding;
        /**
        m_defaultHintingMode sets the default hinting mode, which is used unless the hinting mode
        is set specifically by the FontSpec.
        */
        HintingMode m_defaultHintingMode;
        };

    /** An interface for loading fonts. */
    class FontLoader
        {
        public:
        virtual ReturnCode loadFontData(FontDataId& id,const Utf16Char* path,const Utf16Char* name,const AdvancedFontLoadingParam* advancedParam = 0) = 0;
        virtual ReturnCode loadFontData(FontDataId& id,const Stream& stream,const Utf16Char* name,const AdvancedFontLoadingParam* advancedParam = 0) = 0;
        };

    /** An interface to allow fonts to be loaded on demand. */
    class FontLoaderDelegate
        {
        public:
        /**
        Given some typeface attributes, load any fonts needed
        for the font family by calling functions provided by aFontLoader.
        */
        virtual ReturnCode loadFonts(FontLoader& fontLoader,const Utf16Char* family) = 0;
        };

    /** The engine provides drawing functions. */
    class Engine: public Handle
        {
        public:
        /**
        Create a new engine object. The caller must eventually delete it using Engine::destroy.

        returnCode: 0 if successful, non-zero otherwise
        heap, heapSize: memory to be used for the engine's private heap; if heap is null, the global heap is used        
        */
        TEXT_API_IMPORT static Engine* create(ReturnCode& returnCode,char* heap,int heapSize);

        /**
        Destroy an engine object.

        A destructor is not used because of the need to manage the private heap.
        */
        TEXT_API_IMPORT static void destroy(Engine* engine);

        /**
        Load font data specified by a filename.
        An opaque identifier is returned that can be used to unload the font data.
        The file path is specified as a null-terminated UTF16 string.
        If the supplied name (a null-terminated UTF16 string) is non-null, it is used as the font family name;
        if it is null, the font family name is loaded from the data.
        */
        virtual ReturnCode loadFontData(FontDataId& id,const Utf16Char* path,const Utf16Char* name,const AdvancedFontLoadingParam* advancedParam = 0) = 0;

        /**
        Load font data into the engine.
        An opaque identifier is returned that can be used to unload the font data.
        The stream structure is copied by loadFontData and thus need not remain in existence after the call.
        If the supplied name (a null-terminated UTF16 string) is non-null, it is used as the font family name;
        if it is null, the font family name is loaded from the data.
        */
        virtual ReturnCode loadFontData(FontDataId& id,const Stream& stream,const Utf16Char* name,const AdvancedFontLoadingParam* advancedParam = 0) = 0;

        /** Unload font data. */
        virtual ReturnCode unloadFontData(FontDataId id) = 0;

        /** Create a new font object. The caller must eventually delete it using the ordinary 'delete' operator. */
        virtual Font* createFont(ReturnCode& returnCode,const FontSpec& fontSpec) = 0;

        /**
        Clear the engine's glyph cache.
        */
        virtual int clearGlyphCache() = 0;

        /**
        Specify an object to be used to load fonts on demand.
        If the delegate is specified, FontLoaderDelegate::loadFonts is called when a new font family is needed,
        and is provided with an interface through which fonts may be loaded.
        */
        virtual void setFontLoaderDelegate(FontLoaderDelegate* fontLoaderDelegate) = 0;

        /**
        Supply a list of font family names to be used as substitutes. The first name is the preferred
        one, followed by family names that can be used as substitutes, in order of preference.
        The list is a comma-separated null-terminated UTF16 string. Names must not have leading or trailing spaces
        but may have internal spaces. A sample list: "sans-serif,Helvetica,Arial".
        You can supply as many lists as are needed.

        Names (apart from the first one) may be followed by lists of modifying options, each enclosed in curly braces. For example,
        the list "Calibri,Arial{h=0.9}{w=0.95}" states that when Arial is used as a substitute for Calibri
        it must be rendered at 0.9 times the requested height, and 0.95 times the requested width.
        The width factor is applied after the height factor; thus, it is unnecessary to give a width factor
        unless specifying a condensed or expanded variant.
        Only the options 'h' and 'w' are currently supported.
        
        A list with the same first name as an existing list overrides the existing list.
        A list with only the first name and no substitutes causes any list for that name to be deleted.
        An empty list has no effect.

        Names are compared ignoring case, as in font matching.
        */
        virtual ReturnCode setFontFamilyList(const Utf16Char* fontFamilyList) = 0; 
        
        /** Create a new graphics context object. The caller must eventually delete it using the ordinary 'delete' operator. */
        virtual GraphicsContext* createGraphicsContext(ReturnCode& returnCode,GraphicsContextType type,NativeGraphicsDisplay display) = 0;

        /**
        Create a new graphics context object that optionally treats the native display, surface and context as its own.

        If ownGraphicsState is true it makes the context current, sets the graphics state appropriately (e.g., line join and cap),
        and does not save and restore the native graphics state.

        If ownGraphicsState is false it saves and restores the context and graphics state when graphics operations are performed,
        and does more error checking. This if course is slower.
        
        The caller must eventually delete the graphics context using the ordinary 'delete' operator.
        */
        virtual GraphicsContext* createGraphicsContext(ReturnCode& returnCode,GraphicsContextType type,
                                                       NativeGraphicsDisplay display,
                                                       NativeGraphicsContext context,
                                                       NativeGraphicsSurface surface,
                                                       bool ownGraphicsState) = 0;

        /**
        Draw or measure text. Text is drawn using the graphics context's current fill and stroke colors
        except where they are explicitly overridden by special effects. The fill color is used for
        ordinary glyphs. The stroke color is used when drawing outlines.

        gc: the graphics context. If this is null, text is measured but not drawn.
        font: the font
        text: the text, as a UTF16 string
        textLength: the length of the text in 16-bit words; if negative, the text is null-terminated
        x, y: the origin of the left end of the text
        wrap: if greater than zero, the wrap width. Text drawing or measurement stops before the advance or bounds width exceeds this value.
        param (may be null): optional parameters for glyph effects, modified spacing, etc.
        metrics (may be null): optional returned metrics
        */
        virtual ReturnCode drawText(GraphicsContext* gc,Font& font,const Utf16Char* text,int textLength,
                                    double x,double y,double wrap,const DrawParam* param,TextMetrics* metrics) = 0;

        /**
        Convert an X position relative to the origin of the text to a position in the text.
        It is essential to set param.m_textOrder correctly.

        x: the x position
        textPos: the text position returned
        rounding: how to round to the nearest text position
        font: the font
        text: the text, as a UTF16 string
        textLength: the length of the text in 16-bit words; if negative, the text is null-terminated
        param: parameters for glyph effects, modified spacing, etc.
        */
        virtual ReturnCode xToTextPos(double x,int& textPos,TextPosRounding rounding,
                                      Font& font,const Utf16Char* text,int textLength,const DrawParam& param) = 0;


        /**
        Convert a position in the text to an X position relative to the origin of the text.
        It is essential to set param.m_textOrder correctly.

        textPos: the text position
        x: the x position returned
        font: the font
        text: the text, as a UTF16 string
        textLength: the length of the text in 16-bit words; if negative, the text is null-terminated
        param: parameters for glyph effects, modified spacing, etc.
        */
        virtual ReturnCode textPosToX(int textPos,double &x,
                                      Font& font,const Utf16Char* text,int textLength,const DrawParam& param) = 0;

        /**
        Set the glyph type used by fonts and return the previous method.
        By default, OpenVG glyphs are created, but for debugging purposes
        bitmap glyphs can be used. Fonts use the method in force when the font was created. 
        */
        virtual GlyphType setGlyphType(GlyphType type) = 0;


        /**
        Set the native graphics display, surfaces and context (e.g., the EGL parameters) used for creating
        long-lived resources, that will live as long or longer than the engine, as for example
        OpenVG paths and images.
        */
        virtual ReturnCode setResourceContext(NativeGraphicsDisplay aDisplay,
                                              NativeGraphicsSurface aReadSurface,
                                              NativeGraphicsSurface aDrawSurface,
                                              NativeGraphicsContext aContext) = 0;

        /**
        DEPRECATED: will be removed.

        Set the native graphics context (e.g., the EGL context) used for creating
        long-lived resources, that will live as long or longer than the engine, as for example
        OpenVG paths and images.
        */
        virtual ReturnCode setResourceContext(NativeGraphicsContext aContext) = 0;

        protected:
        Engine() { }
        ~Engine() { }
        
        private:
        Engine(const Engine&);
        void operator=(const Engine&);
        };

    /** Types of break iterators. */
    enum BreakIteratorType
        {
        CharacterBreakIteratorType,
        WordBreakIteratorType,
        LineBreakIteratorType,
        SentenceBreakIteratorType
        };

    /** Break iterators find character, word, line and sentence boundaries. */
    class BreakIterator: public Handle
        {
        public:

        enum
            {
            /** The value returned by next, previous, etc., when no more breaks are available. */
            Done = 0x7fffffff
            };

        /**
        Create a new break iterator object, specifying its type and some text.
        If textLength is negative the text is assumed to be null-terminated.
        */
        TEXT_API_IMPORT static BreakIterator* create(ReturnCode& returnCode,BreakIteratorType type,const Utf16Char* text,int textLength);
        virtual int first() = 0;
        virtual int last() = 0;
        virtual int next() = 0;
        virtual int previous() = 0;
        virtual int pos() = 0;
        virtual int preceding(int pos) = 0;
        virtual int following(int pos) = 0;
        virtual bool isBreak(int pos) = 0;
        /** Set the current position to pos. Clamp pos to a legal position. */
        virtual void setPos(int pos) = 0;
        /**
        Set the text, and set the current position to 0.
        If textLength is negative the text is assumed to be null-terminated.
        */
        virtual void setText(const Utf16Char* text,int textLength) = 0;
        };

    /** A namespace for character functions and enumerations. */
    namespace Char
        {
        /** Unicode bidirectional types */
        enum BidirectionalType
            {
		    L = 1,
		    LRE = 2,
		    LRO = 4,
		    R = 8,
		    AL = 0x10,
		    RLE = 0x20,
		    RLO = 0x40,
		    PDF = 0x80,
		    EN = 0x100,
		    ES = 0x200,
		    ET = 0x400,
		    AN = 0x800,
		    CS = 0x1000,
		    NSM = 0x2000,
		    BN = 0x4000,
		    B = 0x8000,
		    S = 0x10000,
		    WS = 0x20000,
		    ON = 0x40000
            };

        /** Decomposition types. */
	    enum DecompositionType
		    {
		    /** The character is not decomposable. */
		    NoDecomposition,
		    /** The character has a canonical decomposition. */
		    CanoniclDecomposition,
		    /** The character has a compatibility decomposition. */
		    CompatibilityDecomposition
		    };

        /** Character categories. */
        enum Cat
		    {
		    LuCat = 1,
		    LlCat = 2,
		    LtCat = 4,
		    LmCat = 8,
		    LoCat = 0x10,

		    LetterCatMask = LuCat | LlCat | LtCat | LmCat | LoCat, 

		    MnCat = 0x20,
		    McCat = 0x40,
		    MeCat = 0x80,
		    
		    MarkCatMask = MnCat | McCat | MeCat,
		    
		    NdCat = 0x100,
		    NlCat = 0x200,
		    NoCat = 0x400,

		    NumberCatMask = NdCat | NlCat | NoCat,

		    ZsCat = 0x800,
		    ZlCat = 0x1000,
		    ZpCat = 0x2000,

		    SeparatorCatMask = ZsCat | ZlCat | ZpCat,

		    CcCat = 0x4000,
		    CfCat = 0x8000,
		    CsCat = 0x10000,
		    CoCat = 0x20000,
		    CnCat = 0x40000,

		    ControlCatMask = CcCat | CfCat | CsCat | CoCat | CnCat,

		    PcCat = 0x80000,
		    PdCat = 0x100000,
		    PsCat = 0x200000,
		    PeCat = 0x400000,
		    PiCat = 0x800000,
		    PfCat = 0x1000000,
		    PoCat = 0x2000000,

		    PunctCatMask = PcCat | PdCat | PsCat | PeCat | PiCat | PfCat | PoCat,

		    SmCat = 0x4000000,
		    ScCat = 0x8000000,
		    SkCat = 0x10000000,
		    SoCat = 0x20000000,

		    SymbolCatMask = SmCat | ScCat | SkCat | SoCat
		    };

        TEXT_API_IMPORT BidirectionalType bidirectionalType(int c);
        TEXT_API_IMPORT DecompositionType decompositionType(int c);
        TEXT_API_IMPORT Cat category(int c);
        TEXT_API_IMPORT bool isSpace(int c);
        TEXT_API_IMPORT bool isLetter(int c);
        TEXT_API_IMPORT bool isPrintable(int c);
        TEXT_API_IMPORT bool isUpper(int c);
        TEXT_API_IMPORT bool isLower(int c);
        TEXT_API_IMPORT bool isPunct(int c);
        TEXT_API_IMPORT bool isDigit(int c);
        TEXT_API_IMPORT int toLower(int c);
        TEXT_API_IMPORT int toUpper(int c);
        TEXT_API_IMPORT int toTitle(int c);
        TEXT_API_IMPORT int foldCase(int c);
        TEXT_API_IMPORT int mirroredForm(int c);
        TEXT_API_IMPORT int combiningClass(int c);
        }

    } // namespace TextAPI

#endif // #ifdef TEXT_API_H_INCLUDED__
