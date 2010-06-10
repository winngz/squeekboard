/* 
 * Copyright (C) 2010 Daiki Ueno <ueno@unixuser.org>
 * Copyright (C) 2010 Red Hat, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

/**
 * SECTION:eek-keyboard
 * @short_description: Base class of a keyboard
 * @see_also: #EekSection
 *
 * The #EekKeyboardClass class represents a keyboard, which consists
 * of one or more sections of the #EekSectionClass class.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* HAVE_CONFIG_H */

#include "eek-keyboard.h"
#include "eek-section.h"
#include "eek-key.h"

enum {
    PROP_0,
    PROP_GROUP,
    PROP_LEVEL,
    PROP_LAST
};

enum {
    KEY_PRESSED,
    KEY_RELEASED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE (EekKeyboard, eek_keyboard, EEK_TYPE_CONTAINER);

#define EEK_KEYBOARD_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EEK_TYPE_KEYBOARD, EekKeyboardPrivate))


struct _EekKeyboardPrivate
{
    gint group;
    gint level;
    EekLayout *layout;
    gboolean is_realized;
};

struct keysym_index {
    gint group;
    gint level;
};

static void
set_keysym_index_for_key (EekElement *element,
                          gpointer    user_data)
{
    struct keysym_index *ki;

    g_return_if_fail (EEK_IS_KEY(element));

    ki = user_data;
    eek_key_set_keysym_index (EEK_KEY(element), ki->group, ki->level);
}

static void
set_keysym_index_for_section (EekElement *element,
                              gpointer user_data)
{
    eek_container_foreach_child (EEK_CONTAINER(element),
                                 set_keysym_index_for_key,
                                 user_data);
}

static void
eek_keyboard_real_set_keysym_index (EekKeyboard *self,
                                    gint         group,
                                    gint         level)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(self);
    struct keysym_index ki;

    ki.group = priv->group = group;
    ki.level = priv->level = level;

    eek_container_foreach_child (EEK_CONTAINER(self),
                                 set_keysym_index_for_section,
                                 &ki);
}

void
eek_keyboard_real_get_keysym_index (EekKeyboard *self,
                                    gint        *group,
                                    gint        *level)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (group || level);
    if (group)
        *group = priv->group;
    if (level)
        *level = priv->level;
}

static void
key_pressed_event (EekSection  *section,
                   EekKey      *key,
                   EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-pressed", key);
}

static void
key_released_event (EekSection  *section,
                    EekKey      *key,
                    EekKeyboard *keyboard)
{
    g_signal_emit_by_name (keyboard, "key-released", key);
}

static EekSection *
eek_keyboard_real_create_section (EekKeyboard *self)
{
    EekSection *section;

    section = g_object_new (EEK_TYPE_SECTION, NULL);
    g_return_val_if_fail (section, NULL);

    g_signal_connect (section, "key-pressed",
                      G_CALLBACK(key_pressed_event), self);
    g_signal_connect (section, "key-released",
                      G_CALLBACK(key_released_event), self);

    EEK_CONTAINER_GET_CLASS(self)->add_child (EEK_CONTAINER(self),
                                              EEK_ELEMENT(section));
    return section;
}

static void
eek_keyboard_real_set_layout (EekKeyboard *self,
                              EekLayout   *layout)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (EEK_IS_LAYOUT(layout));
    priv->layout = layout;
    g_object_ref_sink (priv->layout);
}

static void
eek_keyboard_real_realize (EekKeyboard *self)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(self);

    g_return_if_fail (priv->layout);
    g_return_if_fail (!priv->is_realized);
    EEK_LAYOUT_GET_IFACE(priv->layout)->apply (priv->layout, self);
    priv->is_realized = TRUE;
}

struct find_key_by_keycode_data {
    EekKey *key;
    guint keycode;
};

static gint
compare_section_by_keycode (EekElement *element, gpointer user_data)
{
    struct find_key_by_keycode_data *data = user_data;

    data->key = eek_section_find_key_by_keycode (EEK_SECTION(element),
                                                 data->keycode);
    if (data->key)
        return 0;
    return -1;
}

static EekKey *
eek_keyboard_real_find_key_by_keycode (EekKeyboard *self,
                                       guint        keycode)
{
    struct find_key_by_keycode_data data;

    data.keycode = keycode;
    if (eek_container_find (EEK_CONTAINER(self),
                            compare_section_by_keycode,
                            &data))
        return data.key;
    return NULL;
}

static void
eek_keyboard_finalize (GObject *object)
{
    EekKeyboardPrivate *priv = EEK_KEYBOARD_GET_PRIVATE(object);

    if (priv->layout)
        g_object_unref (priv->layout);

    G_OBJECT_CLASS(eek_keyboard_parent_class)->finalize (object);
}

static void
eek_keyboard_set_property (GObject    *object,
                           guint       prop_id,
                           const GValue     *value,
                           GParamSpec *pspec)
{
    gint group, level;

    g_return_if_fail (EEK_IS_KEYBOARD(object));
    switch (prop_id) {
    case PROP_GROUP:
        eek_keyboard_get_keysym_index (EEK_KEYBOARD(object), &group, &level);
        eek_keyboard_set_keysym_index (EEK_KEYBOARD(object),
                                       g_value_get_int (value),
                                       level);
        break;
    case PROP_LEVEL:
        eek_keyboard_get_keysym_index (EEK_KEYBOARD(object), &group, &level);
        eek_keyboard_set_keysym_index (EEK_KEYBOARD(object),
                                       group,
                                       g_value_get_int (value));
        break;
    default:
        g_object_set_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_keyboard_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    gint group, level;

    g_return_if_fail (EEK_IS_KEYBOARD(object));
    switch (prop_id) {
    case PROP_GROUP:
        eek_keyboard_get_keysym_index (EEK_KEYBOARD(object), &group, &level);
        g_value_set_int (value, group);
        break;
    case PROP_LEVEL:
        eek_keyboard_get_keysym_index (EEK_KEYBOARD(object), &level, &level);
        g_value_set_int (value, level);
        break;
    default:
        g_object_get_property (object,
                               g_param_spec_get_name (pspec),
                               value);
        break;
    }
}

static void
eek_keyboard_class_init (EekKeyboardClass *klass)
{
    GObjectClass      *gobject_class = G_OBJECT_CLASS (klass);
    GParamSpec        *pspec;

    g_type_class_add_private (gobject_class,
                              sizeof (EekKeyboardPrivate));

    klass->set_keysym_index = eek_keyboard_real_set_keysym_index;
    klass->get_keysym_index = eek_keyboard_real_get_keysym_index;
    klass->create_section = eek_keyboard_real_create_section;
    klass->set_layout = eek_keyboard_real_set_layout;
    klass->realize = eek_keyboard_real_realize;
    klass->find_key_by_keycode = eek_keyboard_real_find_key_by_keycode;

    gobject_class->get_property = eek_keyboard_get_property;
    gobject_class->set_property = eek_keyboard_set_property;
    gobject_class->finalize = eek_keyboard_finalize;

    /**
     * EekKeyboard:group:
     *
     * The group (row) index of symbol matrix of #EekKeyboard.
     */
    pspec = g_param_spec_int ("group",
                              "Group",
                              "Group index of symbol matrix of the keyboard",
                              0, G_MAXINT, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_GROUP,
                                     pspec);

    /**
     * EekKeyboard:level:
     *
     * The level (row) index of symbol matrix of #EekKeyboard.
     */
    pspec = g_param_spec_int ("level",
                              "Level",
                              "Level index of symbol matrix of the keyboard",
                              0, G_MAXINT, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class,
                                     PROP_LEVEL,
                                     pspec);

    signals[KEY_PRESSED] =
        g_signal_new ("key-pressed",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      EEK_TYPE_KEY);

    signals[KEY_RELEASED] =
        g_signal_new ("key-released",
                      G_TYPE_FROM_CLASS(gobject_class),
                      G_SIGNAL_RUN_FIRST,
                      0,
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__OBJECT,
                      G_TYPE_NONE,
                      1,
                      EEK_TYPE_KEY);
}

static void
eek_keyboard_init (EekKeyboard *self)
{
    EekKeyboardPrivate *priv;

    priv = self->priv = EEK_KEYBOARD_GET_PRIVATE(self);
    priv->group = priv->level = 0;
    priv->layout = NULL;
    priv->is_realized = FALSE;
}

/**
 * eek_keyboard_set_keysym_index:
 * @keyboard: an #EekKeyboard
 * @group: row index of the symbol matrix of keys on @keyboard
 * @level: column index of the symbol matrix of keys on @keyboard
 *
 * Select a cell of the symbol matrix of each key on @keyboard.
 */
void
eek_keyboard_set_keysym_index (EekKeyboard *keyboard,
                               gint         group,
                               gint         level)
{
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    EEK_KEYBOARD_GET_CLASS(keyboard)->set_keysym_index (keyboard, group, level);
}

/**
 * eek_keyboard_get_keysym_index:
 * @keyboard: an #EekKeyboard
 * @group: a pointer where row index of the symbol matrix of keys on
 * @keyboard will be stored
 * @level: a pointer where column index of the symbol matrix of keys
 * on @keyboard will be stored
 *
 * Get the current cell position of the symbol matrix of each key on @keyboard.
 */
void
eek_keyboard_get_keysym_index (EekKeyboard *keyboard,
                               gint        *group,
                               gint        *level)
{
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    EEK_KEYBOARD_GET_CLASS(keyboard)->get_keysym_index (keyboard, group, level);
}

/**
 * eek_keyboard_create_section:
 * @keyboard: an #EekKeyboard
 * @name: name of the section
 * @bounds: bounding box of the section
 *
 * Create an #EekSection instance and attach it to @keyboard.
 */
EekSection *
eek_keyboard_create_section (EekKeyboard *keyboard)
{
    EekSection *section;
    g_return_val_if_fail (EEK_IS_KEYBOARD(keyboard), NULL);
    section = EEK_KEYBOARD_GET_CLASS(keyboard)->create_section (keyboard);
    return section;
}

/**
 * eek_keyboard_set_layout:
 * @keyboard: an #EekKeyboard
 * @layout: an #EekLayout
 *
 * Set the layout of @keyboard to @layout.  This actually turns
 * @keyboard to be ready to be drawn on the screen.
 */
void
eek_keyboard_set_layout (EekKeyboard *keyboard,
                         EekLayout   *layout)
{
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    EEK_KEYBOARD_GET_CLASS(keyboard)->set_layout (keyboard, layout);
}

void
eek_keyboard_realize (EekKeyboard *keyboard)
{
    g_return_if_fail (EEK_IS_KEYBOARD(keyboard));
    EEK_KEYBOARD_GET_CLASS(keyboard)->realize (keyboard);
}

EekKey *
eek_keyboard_find_key_by_keycode (EekKeyboard *keyboard,
                                  guint        keycode)
{
    g_return_val_if_fail (EEK_IS_KEYBOARD(keyboard), NULL);
    return EEK_KEYBOARD_GET_CLASS(keyboard)->find_key_by_keycode (keyboard,
                                                                  keycode);
}
