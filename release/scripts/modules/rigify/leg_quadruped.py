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
from rna_prop_ui import rna_idprop_ui_prop_get
from math import pi
from rigify import RigifyError
from rigify_utils import bone_class_instance, copy_bone_simple, add_pole_target_bone, get_side_name, get_base_name
from Mathutils import Vector

METARIG_NAMES = "hips", "thigh", "shin", "foot", "toe"


def metarig_template():
    # generated by rigify.write_meta_rig
    bpy.ops.object.mode_set(mode='EDIT')
    obj = bpy.context.active_object
    arm = obj.data
    bone = arm.edit_bones.new('body')
    bone.head[:] = -0.0728, -0.2427, 0.0000
    bone.tail[:] = -0.0728, -0.2427, 0.2427
    bone.roll = 0.0000
    bone.connected = False
    bone = arm.edit_bones.new('thigh')
    bone.head[:] = 0.0000, 0.0000, -0.0000
    bone.tail[:] = 0.0813, -0.2109, -0.3374
    bone.roll = -0.4656
    bone.connected = False
    bone.parent = arm.edit_bones['body']
    bone = arm.edit_bones.new('shin')
    bone.head[:] = 0.0813, -0.2109, -0.3374
    bone.tail[:] = 0.0714, -0.0043, -0.5830
    bone.roll = -0.2024
    bone.connected = True
    bone.parent = arm.edit_bones['thigh']
    bone = arm.edit_bones.new('foot')
    bone.head[:] = 0.0714, -0.0043, -0.5830
    bone.tail[:] = 0.0929, -0.0484, -0.7652
    bone.roll = -0.3766
    bone.connected = True
    bone.parent = arm.edit_bones['shin']
    bone = arm.edit_bones.new('toe')
    bone.head[:] = 0.0929, -0.0484, -0.7652
    bone.tail[:] = 0.1146, -0.1244, -0.7652
    bone.roll = -0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['foot']

    bpy.ops.object.mode_set(mode='OBJECT')
    pbone = obj.pose.bones['thigh']
    pbone['type'] = 'leg_quadruped'


def metarig_definition(obj, orig_bone_name):
    '''
    The bone given is the first in a chain
    Expects a chain of at least 3 children.
    eg.
        thigh -> shin -> foot -> [toe, heel]
    '''

    bone_definition = []

    orig_bone = obj.data.bones[orig_bone_name]
    orig_bone_parent = orig_bone.parent

    if orig_bone_parent is None:
        raise RigifyError("expected the thigh bone to have a parent hip bone")

    bone_definition.append(orig_bone_parent.name)
    bone_definition.append(orig_bone.name)


    bone = orig_bone
    chain = 0
    while chain < 3: # first 2 bones only have 1 child
        children = bone.children

        if len(children) != 1:
            raise RigifyError("expected the thigh bone to have 3 children without a fork")
        bone = children[0]
        bone_definition.append(bone.name) # shin, foot
        chain += 1

    if len(bone_definition) != len(METARIG_NAMES):
        raise RigifyError("internal problem, expected %d bones" % len(METARIG_NAMES))

    return bone_definition


def ik(obj, bone_definition, base_names, options):
    eb = obj.data.edit_bones
    pb = obj.pose.bones
    arm = obj.data
    bpy.ops.object.mode_set(mode='EDIT')

    # setup the existing bones, use names from METARIG_NAMES
    mt = bone_class_instance(obj, ["hips"])
    mt_chain = bone_class_instance(obj, ["thigh", "shin", "foot", "toe"])

    mt.attr_initialize(METARIG_NAMES, bone_definition)
    mt_chain.attr_initialize(METARIG_NAMES, bone_definition)

    ik_chain = mt_chain.copy(to_fmt="MCH-%s.ik", base_names=base_names)

    ik_chain.thigh_e.connected = False
    ik_chain.thigh_e.parent = mt.hips_e

    ik_chain.foot_e.parent = None
    ik_chain.rename("foot", get_base_name(base_names[bone_definition[3]]) + "_ik" + get_side_name(base_names[bone_definition[3]]))
    ik_chain.rename("toe", get_base_name(base_names[bone_definition[4]]) + "_ik" + get_side_name(base_names[bone_definition[4]]))

    # keep the foot_ik as the parent
    ik_chain.toe_e.connected = False

    # must be after disconnecting the toe
    ik_chain.foot_e.align_orientation(mt_chain.toe_e)

    # children of ik_foot
    ik = bone_class_instance(obj, ["foot_roll", "foot_roll_01", "foot_roll_02", "foot_target"])

    # knee rotator
    knee_rotator = copy_bone_simple(arm, mt_chain.toe, "knee_rotator" + get_side_name(base_names[mt_chain.foot]), parent=True).name
    eb[knee_rotator].connected = False
    eb[knee_rotator].parent = eb[mt.hips]
    eb[knee_rotator].head = eb[ik_chain.thigh].head
    eb[knee_rotator].tail = eb[knee_rotator].head + eb[mt_chain.toe].vector
    eb[knee_rotator].length = eb[ik_chain.thigh].length / 2
    eb[knee_rotator].roll += pi/2
    
    # parent ik leg to the knee rotator
    eb[ik_chain.thigh].parent = eb[knee_rotator]

    # foot roll is an interesting one!
    # plot a vector from the toe bones head, bactwards to the length of the foot
    # then align it with the foot but reverse direction.
    ik.foot_roll_e = copy_bone_simple(arm, mt_chain.toe, get_base_name(base_names[mt_chain.foot]) + "_roll" + get_side_name(base_names[mt_chain.foot]))
    ik.foot_roll = ik.foot_roll_e.name
    ik.foot_roll_e.connected = False
    ik.foot_roll_e.parent = ik_chain.foot_e
    ik.foot_roll_e.head -= mt_chain.toe_e.vector.normalize() * mt_chain.foot_e.length
    ik.foot_roll_e.tail = ik.foot_roll_e.head - (mt_chain.foot_e.vector.normalize() * mt_chain.toe_e.length)
    ik.foot_roll_e.align_roll(mt_chain.foot_e.matrix.rotation_part() * Vector(0.0, 0.0, -1.0))

    # MCH-foot
    ik.foot_roll_01_e = copy_bone_simple(arm, mt_chain.foot, "MCH-" + base_names[mt_chain.foot])
    ik.foot_roll_01 = ik.foot_roll_01_e.name
    ik.foot_roll_01_e.parent = ik_chain.foot_e
    ik.foot_roll_01_e.head, ik.foot_roll_01_e.tail = mt_chain.foot_e.tail, mt_chain.foot_e.head
    ik.foot_roll_01_e.roll = ik.foot_roll_e.roll

    # ik_target, child of MCH-foot
    ik.foot_target_e = copy_bone_simple(arm, mt_chain.foot, "MCH-" + base_names[mt_chain.foot] + "_ik_target")
    ik.foot_target = ik.foot_target_e.name
    ik.foot_target_e.parent = ik.foot_roll_01_e
    ik.foot_target_e.align_orientation(ik_chain.foot_e)
    ik.foot_target_e.length = ik_chain.foot_e.length / 2.0
    ik.foot_target_e.connected = True

    # MCH-foot.02 child of MCH-foot
    ik.foot_roll_02_e = copy_bone_simple(arm, mt_chain.foot, "MCH-%s_02" % base_names[mt_chain.foot])
    ik.foot_roll_02 = ik.foot_roll_02_e.name
    ik.foot_roll_02_e.parent = ik.foot_roll_01_e


    bpy.ops.object.mode_set(mode='OBJECT')

    mt.update()
    mt_chain.update()
    ik.update()
    ik_chain.update()
    
    # Set rotation modes and axis locks
    #pb[knee_rotator].rotation_mode = 'YXZ'
    #pb[knee_rotator].lock_rotation = False, True, False
    pb[knee_rotator].lock_location = True, True, True
    pb[ik.foot_roll].rotation_mode = 'XYZ'
    pb[ik.foot_roll].lock_rotation = False, True, True
    pb[ik_chain.toe].rotation_mode = 'XYZ'
    pb[ik_chain.toe].lock_rotation = False, True, True
    
    # IK switch property
    prop = rna_idprop_ui_prop_get(pb[ik_chain.foot], "ik", create=True)
    pb[ik_chain.foot]["ik"] = 1.0
    prop["soft_min"] = 0.0
    prop["soft_max"] = 1.0
    prop["min"] = 0.0
    prop["max"] = 1.0
    
    ik_driver_path = pb[ik_chain.foot].path_to_id() + '["ik"]'

    # simple constraining of orig bones
    con = mt_chain.thigh_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = ik_chain.thigh
    fcurve = con.driver_add("influence", 0)
    driver = fcurve.driver
    var = driver.variables.new()
    driver.type = 'AVERAGE'
    var.name = "var"
    var.targets[0].id_type = 'OBJECT'
    var.targets[0].id = obj
    var.targets[0].data_path = ik_driver_path

    con = mt_chain.shin_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = ik_chain.shin
    fcurve = con.driver_add("influence", 0)
    driver = fcurve.driver
    var = driver.variables.new()
    driver.type = 'AVERAGE'
    var.name = "var"
    var.targets[0].id_type = 'OBJECT'
    var.targets[0].id = obj
    var.targets[0].data_path = ik_driver_path

    con = mt_chain.foot_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = ik.foot_roll_02
    fcurve = con.driver_add("influence", 0)
    driver = fcurve.driver
    var = driver.variables.new()
    driver.type = 'AVERAGE'
    var.name = "var"
    var.targets[0].id_type = 'OBJECT'
    var.targets[0].id = obj
    var.targets[0].data_path = ik_driver_path

    con = mt_chain.toe_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = ik_chain.toe
    fcurve = con.driver_add("influence", 0)
    driver = fcurve.driver
    var = driver.variables.new()
    driver.type = 'AVERAGE'
    var.name = "var"
    var.targets[0].id_type = 'OBJECT'
    var.targets[0].id = obj
    var.targets[0].data_path = ik_driver_path

    # others...
    con = ik.foot_roll_01_p.constraints.new('COPY_ROTATION')
    con.target = obj
    con.subtarget = ik.foot_roll


    # IK
    con = ik_chain.shin_p.constraints.new('IK')
    con.chain_length = 2
    con.iterations = 500
    con.pole_angle = -90.0 # XXX - in deg!
    con.use_tail = True
    con.use_stretch = True
    con.use_target = True
    con.use_rotation = False
    con.weight = 1.0

    con.target = obj
    con.subtarget = ik.foot_target

    con.pole_target = None

    ik.update()
    ik_chain.update()

    # Set layers of the bones.
    if "ik_layer" in options:
        layer = [n==options["ik_layer"] for n in range(0,32)]
    else:
        layer = list(mt_chain.thigh_b.layer)
    for attr in ik_chain.attr_names:
        obj.data.bones[getattr(ik_chain, attr)].layer = layer
    for attr in ik.attr_names:
        obj.data.bones[getattr(ik, attr)].layer = layer
    obj.data.bones[knee_rotator].layer = layer
    
    return None, ik_chain.thigh, ik_chain.shin, ik_chain.foot, ik_chain.toe
    


def fk(obj, bone_definition, base_names, options):
    eb = obj.data.edit_bones
    pb = obj.pose.bones
    arm = obj.data
    bpy.ops.object.mode_set(mode='EDIT')

    # setup the existing bones, use names from METARIG_NAMES
    mt = bone_class_instance(obj, ["hips"])
    mt_chain = bone_class_instance(obj, ["thigh", "shin", "foot", "toe"])

    mt.attr_initialize(METARIG_NAMES, bone_definition)
    mt_chain.attr_initialize(METARIG_NAMES, bone_definition)
    
    fk_chain = mt_chain.copy(to_fmt="%s", base_names=base_names)
    
    # Create the socket
    socket = copy_bone_simple(arm, mt_chain.thigh, "MCH-leg_socket").name
    eb[socket].parent = eb[mt.hips]
    eb[socket].length = eb[mt_chain.thigh].length / 4
    
    # Create the hinge
    hinge = copy_bone_simple(arm, mt.hips, "MCH-leg_hinge").name
    eb[hinge].length = eb[mt.hips].length / 2
    
    # Make leg child of hinge
    eb[fk_chain.thigh].connected = False
    eb[fk_chain.thigh].parent = eb[hinge]
    
    
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Set rotation modes and axis locks
    pb[fk_chain.shin].rotation_mode = 'XYZ'
    pb[fk_chain.shin].lock_rotation = False, True, True
    
    # Constrain original bones to control bones
    con = mt_chain.thigh_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = fk_chain.thigh
    
    con = mt_chain.shin_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = fk_chain.shin

    con = mt_chain.foot_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = fk_chain.foot

    con = mt_chain.toe_p.constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = fk_chain.toe
    
    # Socket constraint
    con = pb[fk_chain.thigh].constraints.new('COPY_LOCATION')
    con.target = obj
    con.subtarget = socket
    
    # Hinge constraint
    con = pb[hinge].constraints.new('COPY_TRANSFORMS')
    con.target = obj
    con.subtarget = mt.hips
    
    prop = rna_idprop_ui_prop_get(pb[fk_chain.thigh], "hinge", create=True)
    pb[fk_chain.thigh]["hinge"] = 0.0
    prop["soft_min"] = 0.0
    prop["soft_max"] = 1.0
    prop["min"] = 0.0
    prop["max"] = 1.0
    
    hinge_driver_path = pb[fk_chain.thigh].path_to_id() + '["hinge"]'

    fcurve = con.driver_add("influence", 0)
    driver = fcurve.driver
    var = driver.variables.new()
    driver.type = 'AVERAGE'
    var.name = "var"
    var.targets[0].id_type = 'OBJECT'
    var.targets[0].id = obj
    var.targets[0].data_path = hinge_driver_path

    mod = fcurve.modifiers[0]
    mod.poly_order = 1
    mod.coefficients[0] = 1.0
    mod.coefficients[1] = -1.0
    
    return None, fk_chain.thigh, fk_chain.shin, fk_chain.foot, fk_chain.toe




def deform(obj, definitions, base_names, options):
    bpy.ops.object.mode_set(mode='EDIT')

    # Create upper leg bones: two bones, each half of the upper leg.
    uleg1 = copy_bone_simple(obj.data, definitions[1], "DEF-%s.01" % base_names[definitions[1]], parent=True)
    uleg2 = copy_bone_simple(obj.data, definitions[1], "DEF-%s.02" % base_names[definitions[1]], parent=True)
    uleg1.connected = False
    uleg2.connected = False
    uleg2.parent = uleg1
    center = uleg1.center
    uleg1.tail = center
    uleg2.head = center
    
    # Create lower leg bones: two bones, each half of the lower leg.
    lleg1 = copy_bone_simple(obj.data, definitions[2], "DEF-%s.01" % base_names[definitions[2]], parent=True)
    lleg2 = copy_bone_simple(obj.data, definitions[2], "DEF-%s.02" % base_names[definitions[2]], parent=True)
    lleg1.connected = False
    lleg2.connected = False
    lleg2.parent = lleg1
    center = lleg1.center
    lleg1.tail = center
    lleg2.head = center
    
    # Create a bone for the second lower leg deform bone to twist with
    twist = copy_bone_simple(obj.data, lleg2.name, "MCH-leg_twist")
    twist.length /= 4
    twist.connected = False
    twist.parent = obj.data.edit_bones[definitions[3]]
    
    # Create foot bone
    foot = copy_bone_simple(obj.data, definitions[3], "DEF-%s" % base_names[definitions[3]], parent=True)
    
    # Create toe bone
    toe = copy_bone_simple(obj.data, definitions[4], "DEF-%s" % base_names[definitions[4]], parent=True)
    
    # Store names before leaving edit mode
    uleg1_name = uleg1.name
    uleg2_name = uleg2.name
    lleg1_name = lleg1.name
    lleg2_name = lleg2.name
    twist_name = twist.name
    foot_name = foot.name
    toe_name = toe.name
    
    # Leave edit mode
    bpy.ops.object.mode_set(mode='OBJECT')
    
    # Get the pose bones
    uleg1 = obj.pose.bones[uleg1_name]
    uleg2 = obj.pose.bones[uleg2_name]
    lleg1 = obj.pose.bones[lleg1_name]
    lleg2 = obj.pose.bones[lleg2_name]
    foot = obj.pose.bones[foot_name]
    toe = obj.pose.bones[toe_name]
    
    # Upper leg constraints
    con = uleg1.constraints.new('DAMPED_TRACK')
    con.name = "trackto"
    con.target = obj
    con.subtarget = definitions[2]
    
    con = uleg2.constraints.new('COPY_ROTATION')
    con.name = "copy_rot"
    con.target = obj
    con.subtarget = definitions[1]
    
    # Lower leg constraints
    con = lleg1.constraints.new('COPY_ROTATION')
    con.name = "copy_rot"
    con.target = obj
    con.subtarget = definitions[2]
    
    con = lleg2.constraints.new('COPY_ROTATION')
    con.name = "copy_rot"
    con.target = obj
    con.subtarget = twist_name
    
    con = lleg2.constraints.new('DAMPED_TRACK')
    con.name = "trackto"
    con.target = obj
    con.subtarget = definitions[3]
    
    # Foot constraint
    con = foot.constraints.new('COPY_ROTATION')
    con.name = "copy_rot"
    con.target = obj
    con.subtarget = definitions[3]
    
    # Toe constraint
    con = toe.constraints.new('COPY_ROTATION')
    con.name = "copy_rot"
    con.target = obj
    con.subtarget = definitions[4]
    
    bpy.ops.object.mode_set(mode='EDIT')
    return (uleg1_name, uleg2_name, lleg1_name, lleg2_name, foot_name, toe_name, None)
    



def main(obj, bone_definition, base_names, options):
    bones_fk = fk(obj, bone_definition, base_names, options)
    bones_ik = ik(obj, bone_definition, base_names, options)
    deform(obj, bone_definition, base_names, options)
    return bones_ik
