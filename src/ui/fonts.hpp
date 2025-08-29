namespace ui
{
    namespace fonts
    {
        extern GdipFont *bold_font_15;

        extern GdipFont *medium_font_11;
        extern GdipFont *medium_font_12;
        extern GdipFont *medium_font_13;
        extern GdipFont *medium_font_14;
        extern GdipFont *medium_font_15;
        extern GdipFont *medium_font_18;

        void init(struct nk_context *context);

        void use_font(GdipFont *font);
    }
}