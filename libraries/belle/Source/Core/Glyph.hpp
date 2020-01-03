
/* 
    Copyright (c) 2007-2013 William Andrew Burnson.
    Copyright (c) 2013-2020 Nicolas Danet.
    
*/

/* < http://opensource.org/licenses/BSD-2-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

namespace belle {

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

class Glyph {

friend class  Typeface;
friend struct FTCallback;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    class Kerning {
    
    public:
        Kerning (unicode character = 0x0000, double offset = 0.0) : next_ (character), horizontal_ (offset)
        {
        }
    
    #if PRIM_CPP11

    public:
        Kerning (const Kerning&) = default;
        Kerning (Kerning&&) = default;
        Kerning& operator = (const Kerning&) = default;
        Kerning& operator = (Kerning&&) = default;

    #endif

    public:
        unicode next_;
        double horizontal_;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

private:
    Glyph() : character_ (0x0000), advance_ (0.0) 
    {
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PRIM_CPP11

public:
    Glyph (const Glyph&) = delete;
    Glyph& operator = (const Glyph&) = delete;

#else
    
private:
    Glyph (const Glyph&);
    Glyph& operator = (const Glyph&);

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    unicode getCharacter() const
    {
        return character_;
    }

    double getAdvance() const
    {
        return advance_;
    }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

public:
    double getKerningWith (unicode character) const
    {
        for (int i = 0; i < kerning_.size(); ++i) {
            if (kerning_[i].next_ == character) { return kerning_[i].horizontal_; }
        }

        return 0.0;
    }
    
    const Path& getPath() const
    {
        return path_;
    }

private:
    unicode character_;
    double advance_;
    Array < Kerning > kerning_;
    Path path_;

private:
    PRIM_LEAK_DETECTOR (Glyph)
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

} // namespace belle

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
