namespace ui
{
    namespace styles
    {
        extern struct nk_style_button titlebar_button;
        extern struct nk_style_button load_button;
        extern struct nk_style_button mod_info_button;
        extern struct nk_style_button slider_button;
        extern struct nk_style_button social_button;
        extern struct nk_style_button mod_select_button;
        extern struct nk_style_button mod_select_button_faded;
        extern struct nk_style_button mod_select_entry_button;

        void init(struct nk_context *context);
    }
}