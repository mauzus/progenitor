/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_Config (void)
{
  GtkWidget *Config;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *GtkEntry_Iso;
  GtkWidget *button5;
  GtkWidget *hbox2;
  GtkWidget *GtkProgressBar_Progress;
  GtkWidget *button6;
  GtkWidget *hbox4;
  GtkWidget *label2;
  GtkWidget *GtkCombo_Method;
  GList *GtkCombo_Method_items = NULL;
  GtkWidget *combo_entry1;
  GtkWidget *hbuttonbox2;
  GtkWidget *GtkButton_Compress;
  GtkWidget *GtkButton_Decompress;
  GtkWidget *hbox3;
  GtkWidget *label1;
  GtkWidget *GtkEntry_CdDev;
  GtkWidget *hbuttonbox3;
  GtkWidget *GtkButton_Create;
  GtkWidget *GtkButton_CreateZ;
  GtkWidget *checkBlockDump;
  GtkWidget *hbuttonbox1;
  GtkWidget *button1;
  GtkWidget *button2;

  Config = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (Config, "Config");
  gtk_container_set_border_width (GTK_CONTAINER (Config), 5);
  gtk_window_set_title (GTK_WINDOW (Config), _("CDVD Config Dialog"));

  vbox1 = gtk_vbox_new (FALSE, 5);
  gtk_widget_set_name (vbox1, "vbox1");
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (Config), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

  hbox1 = gtk_hbox_new (FALSE, 10);
  gtk_widget_set_name (hbox1, "hbox1");
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  GtkEntry_Iso = gtk_entry_new ();
  gtk_widget_set_name (GtkEntry_Iso, "GtkEntry_Iso");
  gtk_widget_show (GtkEntry_Iso);
  gtk_box_pack_start (GTK_BOX (hbox1), GtkEntry_Iso, TRUE, TRUE, 0);

  button5 = gtk_button_new_with_mnemonic (_("Select Iso"));
  gtk_widget_set_name (button5, "button5");
  gtk_widget_show (button5);
  gtk_box_pack_start (GTK_BOX (hbox1), button5, FALSE, FALSE, 0);

  hbox2 = gtk_hbox_new (FALSE, 10);
  gtk_widget_set_name (hbox2, "hbox2");
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 0);

  GtkProgressBar_Progress = gtk_progress_bar_new ();
  gtk_widget_set_name (GtkProgressBar_Progress, "GtkProgressBar_Progress");
  gtk_widget_show (GtkProgressBar_Progress);
  gtk_box_pack_start (GTK_BOX (hbox2), GtkProgressBar_Progress, TRUE, FALSE, 0);

  button6 = gtk_button_new_with_mnemonic (_("Stop"));
  gtk_widget_set_name (button6, "button6");
  gtk_widget_show (button6);
  gtk_box_pack_end (GTK_BOX (hbox2), button6, FALSE, FALSE, 0);

  hbox4 = gtk_hbox_new (FALSE, 5);
  gtk_widget_set_name (hbox4, "hbox4");
  gtk_widget_show (hbox4);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox4, TRUE, TRUE, 0);

  label2 = gtk_label_new (_("Compression Method:"));
  gtk_widget_set_name (label2, "label2");
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox4), label2, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_CENTER);

  GtkCombo_Method = gtk_combo_new ();
  g_object_set_data (G_OBJECT (GTK_COMBO (GtkCombo_Method)->popwin),
                     "GladeParentKey", GtkCombo_Method);
  gtk_widget_set_name (GtkCombo_Method, "GtkCombo_Method");
  gtk_widget_show (GtkCombo_Method);
  gtk_box_pack_start (GTK_BOX (hbox4), GtkCombo_Method, TRUE, FALSE, 0);
  GtkCombo_Method_items = g_list_append (GtkCombo_Method_items, (gpointer) "");
  gtk_combo_set_popdown_strings (GTK_COMBO (GtkCombo_Method), GtkCombo_Method_items);
  g_list_free (GtkCombo_Method_items);

  combo_entry1 = GTK_COMBO (GtkCombo_Method)->entry;
  gtk_widget_set_name (combo_entry1, "combo_entry1");
  gtk_widget_show (combo_entry1);

  hbuttonbox2 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox2, "hbuttonbox2");
  gtk_widget_show (hbuttonbox2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox2, TRUE, TRUE, 0);

  GtkButton_Compress = gtk_button_new_with_mnemonic (_("Compress Iso"));
  gtk_widget_set_name (GtkButton_Compress, "GtkButton_Compress");
  gtk_widget_show (GtkButton_Compress);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), GtkButton_Compress);
  GTK_WIDGET_SET_FLAGS (GtkButton_Compress, GTK_CAN_DEFAULT);

  GtkButton_Decompress = gtk_button_new_with_mnemonic (_("Decompress Iso"));
  gtk_widget_set_name (GtkButton_Decompress, "GtkButton_Decompress");
  gtk_widget_show (GtkButton_Decompress);
  gtk_container_add (GTK_CONTAINER (hbuttonbox2), GtkButton_Decompress);
  GTK_WIDGET_SET_FLAGS (GtkButton_Decompress, GTK_CAN_DEFAULT);

  hbox3 = gtk_hbox_new (FALSE, 5);
  gtk_widget_set_name (hbox3, "hbox3");
  gtk_widget_show (hbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, FALSE, FALSE, 0);

  label1 = gtk_label_new (_("Cdrom Device: "));
  gtk_widget_set_name (label1, "label1");
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox3), label1, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_CENTER);

  GtkEntry_CdDev = gtk_entry_new ();
  gtk_widget_set_name (GtkEntry_CdDev, "GtkEntry_CdDev");
  gtk_widget_show (GtkEntry_CdDev);
  gtk_box_pack_start (GTK_BOX (hbox3), GtkEntry_CdDev, TRUE, TRUE, 0);

  hbuttonbox3 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox3, "hbuttonbox3");
  gtk_widget_show (hbuttonbox3);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox3, TRUE, TRUE, 0);

  GtkButton_Create = gtk_button_new_with_mnemonic (_("Create Iso"));
  gtk_widget_set_name (GtkButton_Create, "GtkButton_Create");
  gtk_widget_show (GtkButton_Create);
  gtk_container_add (GTK_CONTAINER (hbuttonbox3), GtkButton_Create);
  GTK_WIDGET_SET_FLAGS (GtkButton_Create, GTK_CAN_DEFAULT);

  GtkButton_CreateZ = gtk_button_new_with_mnemonic (_("Create Compressed Iso"));
  gtk_widget_set_name (GtkButton_CreateZ, "GtkButton_CreateZ");
  gtk_widget_show (GtkButton_CreateZ);
  gtk_container_add (GTK_CONTAINER (hbuttonbox3), GtkButton_CreateZ);
  GTK_WIDGET_SET_FLAGS (GtkButton_CreateZ, GTK_CAN_DEFAULT);

  checkBlockDump = gtk_check_button_new_with_mnemonic (_("Create a dump of the running iso"));
  gtk_widget_set_name (checkBlockDump, "checkBlockDump");
  gtk_widget_show (checkBlockDump);
  gtk_box_pack_start (GTK_BOX (vbox1), checkBlockDump, FALSE, FALSE, 0);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_set_name (hbuttonbox1, "hbuttonbox1");
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbuttonbox1, TRUE, TRUE, 0);

  button1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_set_name (button1, "button1");
  gtk_widget_show (button1);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button1);
  GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

  button2 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_set_name (button2, "button2");
  gtk_widget_show (button2);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), button2);
  GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) button5, "clicked",
                    G_CALLBACK (OnFileSel),
                    NULL);
  g_signal_connect ((gpointer) button6, "clicked",
                    G_CALLBACK (OnStop),
                    NULL);
  g_signal_connect ((gpointer) GtkButton_Compress, "clicked",
                    G_CALLBACK (OnCompress),
                    NULL);
  g_signal_connect ((gpointer) GtkButton_Decompress, "clicked",
                    G_CALLBACK (OnDecompress),
                    NULL);
  g_signal_connect ((gpointer) GtkButton_Create, "clicked",
                    G_CALLBACK (OnCreate),
                    NULL);
  g_signal_connect ((gpointer) GtkButton_CreateZ, "clicked",
                    G_CALLBACK (OnCreateZ),
                    NULL);
  g_signal_connect ((gpointer) button1, "clicked",
                    G_CALLBACK (OnOk),
                    NULL);
  g_signal_connect ((gpointer) button2, "clicked",
                    G_CALLBACK (OnCancel),
                    NULL);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (Config, Config, "Config");
  GLADE_HOOKUP_OBJECT (Config, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (Config, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (Config, GtkEntry_Iso, "GtkEntry_Iso");
  GLADE_HOOKUP_OBJECT (Config, button5, "button5");
  GLADE_HOOKUP_OBJECT (Config, hbox2, "hbox2");
  GLADE_HOOKUP_OBJECT (Config, GtkProgressBar_Progress, "GtkProgressBar_Progress");
  GLADE_HOOKUP_OBJECT (Config, button6, "button6");
  GLADE_HOOKUP_OBJECT (Config, hbox4, "hbox4");
  GLADE_HOOKUP_OBJECT (Config, label2, "label2");
  GLADE_HOOKUP_OBJECT (Config, GtkCombo_Method, "GtkCombo_Method");
  GLADE_HOOKUP_OBJECT (Config, combo_entry1, "combo_entry1");
  GLADE_HOOKUP_OBJECT (Config, hbuttonbox2, "hbuttonbox2");
  GLADE_HOOKUP_OBJECT (Config, GtkButton_Compress, "GtkButton_Compress");
  GLADE_HOOKUP_OBJECT (Config, GtkButton_Decompress, "GtkButton_Decompress");
  GLADE_HOOKUP_OBJECT (Config, hbox3, "hbox3");
  GLADE_HOOKUP_OBJECT (Config, label1, "label1");
  GLADE_HOOKUP_OBJECT (Config, GtkEntry_CdDev, "GtkEntry_CdDev");
  GLADE_HOOKUP_OBJECT (Config, hbuttonbox3, "hbuttonbox3");
  GLADE_HOOKUP_OBJECT (Config, GtkButton_Create, "GtkButton_Create");
  GLADE_HOOKUP_OBJECT (Config, GtkButton_CreateZ, "GtkButton_CreateZ");
  GLADE_HOOKUP_OBJECT (Config, checkBlockDump, "checkBlockDump");
  GLADE_HOOKUP_OBJECT (Config, hbuttonbox1, "hbuttonbox1");
  GLADE_HOOKUP_OBJECT (Config, button1, "button1");
  GLADE_HOOKUP_OBJECT (Config, button2, "button2");

  return Config;
}

