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
from rigify import get_layer_dict
from rigify_utils import bone_class_instance, copy_bone_simple

METARIG_NAMES = tuple()
RIG_TYPE = "stretch_twist"

# TODO
#def metarig_template():
#    # generated by rigify.write_meta_rig
#    bpy.ops.object.mode_set(mode='EDIT')
#    obj = bpy.context.active_object
#    arm = obj.data
#    bone = arm.edit_bones.new('Bone')
#    bone.head[:] = 0.0000, 0.0000, 0.0000
#    bone.tail[:] = 0.0000, 0.0000, 1.0000
#    bone.roll = 0.0000
#    bone.connected = False
#
#    bpy.ops.object.mode_set(mode='OBJECT')
#    pbone = obj.pose.bones['Bone']
#    pbone['type'] = 'copy'

bool_map = {0:False, 1:True,
            0.0:False, 1.0:True,
            "false":False, "true":True,
            "False":False, "True":True,
            "no":False, "yes":True,
            "No":False, "Yes":True}

def metarig_definition(obj, orig_bone_name):
    return (orig_bone_name,)




def main(obj, bone_definition, base_names, options):
    """ A dual-bone stretchy bone setup.  Each half follows the twist of the
        bone on its side.
        Deformation only (no controls).
    """
    # Verify required parameter
    if "to" not in options:
        raise RigifyError("'%s' rig type requires a 'to' parameter (bone: %s)" % (RIG_TYPE, base_names[0]))
    if type(options["to"]) is not str:
        raise RigifyError("'%s' rig type 'to' parameter must be a string (bone: %s)" % (RIG_TYPE, base_names[0]))
    if ("ORG-" + options["to"]) not in obj.data.bones:
        raise RigifyError("'%s' rig type 'to' parameter must name a bone in the metarig (bone: %s)" % (RIG_TYPE, base_names[0]))
    
    preserve_volume = None
    # Check optional parameter
    if "preserve_volume" in options:
        try:
            preserve_volume = bool_map[options["preserve_volume"]]
        except KeyError:
            preserve_volume = False
    
    eb = obj.data.edit_bones
    bb = obj.data.bones
    pb = obj.pose.bones
        
    bpy.ops.object.mode_set(mode='EDIT')
    arm = obj.data
    
    mbone1 = bone_definition[0]
    mbone2 = "ORG-" + options["to"]
    
    bone_e = copy_bone_simple(obj.data, mbone1, "MCH-%s" % base_names[bone_definition[0]])
    bone_e.connected = False
    bone_e.parent = None
    bone_e.head = (eb[mbone1].head + eb[mbone2].head) / 2
    bone_e.tail = (bone_e.head[0], bone_e.head[1], bone_e.head[2]+0.1)
    mid_bone = bone_e.name
    
    bone_e = copy_bone_simple(obj.data, mbone1, "DEF-%s.01" % base_names[bone_definition[0]])
    bone_e.connected = False
    bone_e.parent = eb[mbone1]
    bone_e.tail = eb[mid_bone].head
    bone1 = bone_e.name
    
    bone_e = copy_bone_simple(obj.data, mbone2, "DEF-%s.02" % base_names[bone_definition[0]])
    bone_e.connected = False
    bone_e.parent = eb[mbone2]
    bone_e.tail = eb[mid_bone].head
    bone2 = bone_e.name


    
    bpy.ops.object.mode_set(mode='OBJECT')

    # Constraints
    
    # Mid bone
    con = pb[mid_bone].constraints.new('COPY_LOCATION')
    con.target = obj
    con.subtarget = mbone1
    
    con = pb[mid_bone].constraints.new('COPY_LOCATION')
    con.target = obj
    con.subtarget = mbone2
    con.influence = 0.5
    
    # Bone 1
    con = pb[bone1].constraints.new('DAMPED_TRACK')
    con.target = obj
    con.subtarget = mid_bone
    
    con = pb[bone1].constraints.new('STRETCH_TO')
    con.target = obj
    con.subtarget = mid_bone
    con.original_length = bb[bone1].length
    if preserve_volume:
        con.volume = 'VOLUME_XZX'
    else:
        con.volume = 'NO_VOLUME'
        
    # Bone 2
    con = pb[bone2].constraints.new('DAMPED_TRACK')
    con.target = obj
    con.subtarget = mid_bone
    
    con = pb[bone2].constraints.new('STRETCH_TO')
    con.target = obj
    con.subtarget = mid_bone
    con.original_length = bb[bone2].length
    if preserve_volume:
        con.volume = 'VOLUME_XZX'
    else:
        con.volume = 'NO_VOLUME'

    return tuple()

