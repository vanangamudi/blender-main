# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
# ##### END GPL LICENSE BLOCK #####

# <pep8 compliant>
import bpy


class TIME_HT_header(bpy.types.Header):
    bl_space_type = 'TIMELINE'

    def draw(self, context):
        layout = self.layout

        scene = context.scene
        tools = context.tool_settings
        screen = context.screen

        row = layout.row(align=True)
        row.template_header()

        if context.area.show_menus:
            sub = row.row(align=True)
            sub.itemM("TIME_MT_view")
            sub.itemM("TIME_MT_frame")
            sub.itemM("TIME_MT_playback")

        layout.itemR(scene, "use_preview_range", text="PR")

        row = layout.row(align=True)
        if not scene.use_preview_range:
            row.itemR(scene, "start_frame", text="Start")
            row.itemR(scene, "end_frame", text="End")
        else:
            row.itemR(scene, "preview_range_start_frame", text="Start")
            row.itemR(scene, "preview_range_end_frame", text="End")

        layout.itemR(scene, "current_frame", text="")

        layout.itemS()

        row = layout.row(align=True)
        row.item_booleanO("screen.frame_jump", "end", False, text="", icon='ICON_REW')
        row.item_booleanO("screen.keyframe_jump", "next", False, text="", icon='ICON_PREV_KEYFRAME')
        if not screen.animation_playing:
            row.item_booleanO("screen.animation_play", "reverse", True, text="", icon='ICON_PLAY_REVERSE')
            row.itemO("screen.animation_play", text="", icon='ICON_PLAY')
        else:
            sub = row.row()
            sub.scale_x = 2.0
            sub.itemO("screen.animation_play", text="", icon='ICON_PAUSE')
        row.item_booleanO("screen.keyframe_jump", "next", True, text="", icon='ICON_NEXT_KEYFRAME')
        row.item_booleanO("screen.frame_jump", "end", True, text="", icon='ICON_FF')

        row = layout.row(align=True)
        row.itemR(tools, "enable_auto_key", text="", toggle=True, icon='ICON_REC')
        if screen.animation_playing and tools.enable_auto_key:
            subsub = row.row()
            subsub.itemR(tools, "record_with_nla", toggle=True)

        layout.itemR(scene, "sync_audio", text="", toggle=True, icon='ICON_SPEAKER')

        layout.itemS()

        row = layout.row(align=True)
        row.item_pointerR(scene, "active_keying_set", scene, "keying_sets", text="")
        row.itemO("anim.insert_keyframe", text="", icon='ICON_KEY_HLT')
        row.itemO("anim.delete_keyframe", text="", icon='ICON_KEY_DEHLT')


class TIME_MT_view(bpy.types.Menu):
    bl_label = "View"

    def draw(self, context):
        layout = self.layout

        st = context.space_data

        layout.itemO("anim.time_toggle")

        layout.itemS()

        layout.itemR(st, "only_selected")


class TIME_MT_frame(bpy.types.Menu):
    bl_label = "Frame"

    def draw(self, context):
        layout = self.layout
        # tools = context.tool_settings

        layout.itemO("marker.add", text="Add Marker")
        layout.itemO("marker.duplicate", text="Duplicate Marker")
        layout.itemO("marker.move", text="Grab/Move Marker")
        layout.itemO("marker.delete", text="Delete Marker")
        layout.itemL(text="ToDo: Name Marker")

        layout.itemS()

        layout.itemO("time.start_frame_set")
        layout.itemO("time.end_frame_set")

        layout.itemS()

        sub = layout.row()
        #sub.active = tools.enable_auto_key
        sub.itemM("TIME_MT_autokey")


class TIME_MT_playback(bpy.types.Menu):
    bl_label = "Playback"

    def draw(self, context):
        layout = self.layout

        st = context.space_data
        scene = context.scene

        layout.itemR(st, "play_top_left")
        layout.itemR(st, "play_all_3d")
        layout.itemR(st, "play_anim")
        layout.itemR(st, "play_buttons")
        layout.itemR(st, "play_image")
        layout.itemR(st, "play_sequencer")
        layout.itemR(st, "play_nodes")

        layout.itemS()

        layout.itemR(st, "continue_physics")

        layout.itemS()

        layout.itemR(scene, "sync_audio", icon='ICON_SPEAKER')
        layout.itemR(scene, "mute_audio")
        layout.itemR(scene, "scrub_audio")


class TIME_MT_autokey(bpy.types.Menu):
    bl_label = "Auto-Keyframing Mode"

    def draw(self, context):
        layout = self.layout
        tools = context.tool_settings

        layout.active = tools.enable_auto_key

        layout.item_enumR(tools, "autokey_mode", 'ADD_REPLACE_KEYS')
        layout.item_enumR(tools, "autokey_mode", 'REPLACE_KEYS')

bpy.types.register(TIME_HT_header)
bpy.types.register(TIME_MT_view)
bpy.types.register(TIME_MT_frame)
bpy.types.register(TIME_MT_autokey)
bpy.types.register(TIME_MT_playback)
