/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 * 
 * Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 * 
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 * 
 * Contributor(s):
 * 
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder. 
 */
#define Uses_SCIM_CONFIG_BASE

#include <gtk/gtk.h>

#include <scim.h>
#include <gtk/scimkeyselection.h>

#include "sunpinyin_private.h"
#include "sunpinyin_imengine_config_keys.h"

using namespace scim;

#define scim_module_init sunpinyin_imengine_setup_LTX_scim_module_init
#define scim_module_exit sunpinyin_imengine_setup_LTX_scim_module_exit

#define scim_setup_module_create_ui       sunpinyin_imengine_setup_LTX_scim_setup_module_create_ui
#define scim_setup_module_get_category    sunpinyin_imengine_setup_LTX_scim_setup_module_get_category
#define scim_setup_module_get_name        sunpinyin_imengine_setup_LTX_scim_setup_module_get_name
#define scim_setup_module_get_description sunpinyin_imengine_setup_LTX_scim_setup_module_get_description
#define scim_setup_module_load_config     sunpinyin_imengine_setup_LTX_scim_setup_module_load_config
#define scim_setup_module_save_config     sunpinyin_imengine_setup_LTX_scim_setup_module_save_config
#define scim_setup_module_query_changed   sunpinyin_imengine_setup_LTX_scim_setup_module_query_changed

static GtkWidget * input_style_combo = NULL;
static GtkWidget * charset_combo = NULL;
static GtkWidget * minus_pageup_button = NULL;
static GtkWidget * bracket_pageup_button = NULL;
static GtkWidget * comma_pageup_button = NULL;
static GtkWidget * memory_power_button = NULL;

static GtkWidget * create_setup_window ();
static void        load_config (const ConfigPointer &config);
static void        save_config (const ConfigPointer &config);
static bool        query_changed ();

// Module Interface.
extern "C" {
    void scim_module_init (void)
    {
        bindtextdomain (GETTEXT_PACKAGE, SCIM_SUNPINYIN_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
    }

    GtkWidget * scim_setup_module_create_ui (void)
    {
        static GtkWidget *setup_ui = NULL;
        if (setup_ui == NULL)
            setup_ui = create_setup_window ();
        return setup_ui;
    }

    String scim_setup_module_get_category (void)
    {
        return String ("IMEngine");
    }

    String scim_setup_module_get_name (void)
    {
        return String (_("SunPinyin"));
    }

    String scim_setup_module_get_description (void)
    {
        return String (_("A Statistical Language Model Based Intelligent IMEngine Module for Simplified Chinese."));
    }

    void scim_setup_module_load_config (const ConfigPointer &config)
    {
        load_config (config);
    }

    void scim_setup_module_save_config (const ConfigPointer &config)
    {
        save_config (config);
    }

    bool scim_setup_module_query_changed ()
    {
        return query_changed ();
    }
} // extern "C"

static bool __have_changed                 = false;

static void
on_value_changed(GtkWidget *  widget,
                 gpointer     user_data)
{
    __have_changed = true;
}

static GtkWidget *
create_options_page(GtkTooltips *tooltips)
{
    GtkWidget *vbox;
    GtkWidget *label;
    GtkWidget *button;

    vbox = gtk_vbox_new (FALSE, 12);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);

    GtkWidget *table = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, TRUE, 0);

    label = gtk_label_new (_("Input Style:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);


    GtkWidget *combo_box = gtk_combo_box_new_text();
    gtk_table_attach (GTK_TABLE (table), combo_box, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("Classic Style"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("Instant Style"));
    gtk_tooltips_set_tip(tooltips, combo_box, _("You may feel more comfortable in classic style, "
                                                "if you are used to input methods like scim-pinyin."
                                                " In instant style, the most possible candidate "
                                                "word will show up in the preedit area right after "
                                                "its pinyin is input."), NULL);
    g_signal_connect(G_OBJECT(combo_box), "changed",
                     G_CALLBACK(on_value_changed), NULL);
    input_style_combo = combo_box;

    label = gtk_label_new (_("Character Set:"));
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);

    combo_box = gtk_combo_box_new_text();
    gtk_table_attach (GTK_TABLE (table), combo_box, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 4, 4);
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("GB2312"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), _("GBK"));
    gtk_tooltips_set_tip(tooltips, combo_box, _("Choosing GBK over GB2312 will trade some speed "
                                                "with a larger charset in which I search candidate "
                                                "words/characters for you."), NULL);
    g_signal_connect(G_OBJECT(combo_box), "changed",
                     G_CALLBACK(on_value_changed), NULL);
    charset_combo = combo_box;

    button = gtk_check_button_new_with_mnemonic (_("Use -/= for paging down/up"));
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_value_changed), NULL);
    minus_pageup_button = button;

    button = gtk_check_button_new_with_mnemonic (_("Use [/] for paging down/up"));
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_value_changed), NULL);
    bracket_pageup_button = button;

    button = gtk_check_button_new_with_mnemonic (_("Use ,/. for paging down/up"));
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(button), "toggled",
                     G_CALLBACK(on_value_changed), NULL);
    comma_pageup_button = button;

    // MemoryPower
    GtkWidget *hbox;
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox);
    gtk_box_pack_start (GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    label = gtk_label_new (NULL);
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), _("M_emory power:"));
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_misc_set_padding (GTK_MISC (label), 2, 0);

    button = gtk_spin_button_new_with_range (0, 10, 1);
    gtk_tooltips_set_tip(tooltips, button, _("The larger this number is, the faster I "
                                             "memorize/forget new words."), NULL);
    g_signal_connect (G_OBJECT(button), "value_changed",
                      G_CALLBACK(on_value_changed), NULL);
    gtk_widget_show (button);
    gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (button), TRUE);
    gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (button), TRUE);
    gtk_spin_button_set_digits (GTK_SPIN_BUTTON (button), 0);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);
    memory_power_button = button;
    
    return vbox;
}

static GtkWidget *
create_setup_window ()
{
    GtkWidget *notebook;
    GtkWidget *label;
    GtkWidget *page;
    GtkTooltips *tooltips;

    // Create the shared tooltips.
    tooltips = gtk_tooltips_new ();

    notebook = gtk_notebook_new ();

    // Create the option page. 
    page = create_options_page(tooltips);
    label = gtk_label_new (_("Options"));
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), page, label);

    gtk_notebook_set_current_page(GTK_NOTEBOOK (notebook), 0);

    gtk_widget_show_all(notebook);
    return notebook;
}

static void
load_config (const ConfigPointer &config)
{
    if (config.null())
        return;

    int viewtype = config->read (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_VIEW_TYPE), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX(input_style_combo), viewtype);

    int charset  = config->read (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_CHARHSET), 1);
    gtk_combo_box_set_active (GTK_COMBO_BOX(charset_combo), charset);

    bool stat;
    stat = config->read (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_PAGE_MINUS), true);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(minus_pageup_button), stat);

    stat = config->read (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_PAGE_BRACKET), true);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bracket_pageup_button), stat);

    stat = config->read (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_PAGE_COMMA), false);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(comma_pageup_button), stat);

    gint value;
    value = config->read (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_MEMORY_POWER), 5);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(memory_power_button), value);

    __have_changed = false;
}

static void
save_config (const ConfigPointer &config)
{
    if (config.null())
        return;

    int no = gtk_combo_box_get_active(GTK_COMBO_BOX(input_style_combo));
    config->write (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_VIEW_TYPE), no);

    no = gtk_combo_box_get_active(GTK_COMBO_BOX(charset_combo));
    config->write (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_CHARHSET), no);

    gboolean stat;
    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(minus_pageup_button));
    config->write (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_PAGE_MINUS), (bool)stat);

    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bracket_pageup_button));
    config->write (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_PAGE_BRACKET), (bool)stat);

    stat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(comma_pageup_button));
    config->write (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_PAGE_COMMA), (bool)stat);

    gint value;
    value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(memory_power_button));
    config->write (String (SCIM_CONFIG_IMENGINE_SUNPINYIN_USER_MEMORY_POWER), value);
    __have_changed = false;
}

bool
query_changed ()
{
    return __have_changed;
}
