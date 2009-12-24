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

# <pep8-80 compliant>
import bpy
import Mathutils
from math import cos, sin, pi

# could this be stored elsewhere?

def metarig_template():
    # generated by rigify.write_meta_rig
    bpy.ops.object.mode_set(mode='EDIT')
    obj = bpy.context.active_object
    arm = obj.data
    bone = arm.edit_bones.new('root')
    bone.head[:] = 0.0000, 0.0000, 0.0000
    bone.tail[:] = 0.0000, 0.4000, 0.0000
    bone.roll = 0.0000
    bone.connected = False
    bone = arm.edit_bones.new('pelvis')
    bone.head[:] = -0.0000, -0.0145, 1.1263
    bone.tail[:] = -0.0000, -0.0145, 0.9563
    bone.roll = 3.1416
    bone.connected = False
    bone.parent = arm.edit_bones['root']
    bone = arm.edit_bones.new('torso')
    bone.head[:] = -0.0000, -0.0145, 1.1263
    bone.tail[:] = -0.0000, -0.0145, 1.2863
    bone.roll = 3.1416
    bone.connected = False
    bone.parent = arm.edit_bones['pelvis']
    bone = arm.edit_bones.new('spine.01')
    bone.head[:] = 0.0000, 0.0394, 0.9688
    bone.tail[:] = -0.0000, -0.0145, 1.1263
    bone.roll = 0.0000
    bone.connected = False
    bone.parent = arm.edit_bones['torso']
    bone = arm.edit_bones.new('spine.02')
    bone.head[:] = -0.0000, -0.0145, 1.1263
    bone.tail[:] = -0.0000, -0.0213, 1.2884
    bone.roll = -0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['spine.01']
    bone = arm.edit_bones.new('thigh.L')
    bone.head[:] = 0.0933, -0.0421, 1.0434
    bone.tail[:] = 0.0933, -0.0516, 0.5848
    bone.roll = 0.0000
    bone.connected = False
    bone.parent = arm.edit_bones['spine.01']
    bone = arm.edit_bones.new('thigh.R')
    bone.head[:] = -0.0933, -0.0421, 1.0434
    bone.tail[:] = -0.0933, -0.0516, 0.5848
    bone.roll = -0.0000
    bone.connected = False
    bone.parent = arm.edit_bones['spine.01']
    bone = arm.edit_bones.new('spine.03')
    bone.head[:] = -0.0000, -0.0213, 1.2884
    bone.tail[:] = -0.0000, 0.0160, 1.3705
    bone.roll = -0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['spine.02']
    bone = arm.edit_bones.new('shin.L')
    bone.head[:] = 0.0933, -0.0516, 0.5848
    bone.tail[:] = 0.0915, 0.0100, 0.1374
    bone.roll = 0.0034
    bone.connected = True
    bone.parent = arm.edit_bones['thigh.L']
    bone = arm.edit_bones.new('shin.R')
    bone.head[:] = -0.0933, -0.0516, 0.5848
    bone.tail[:] = -0.0915, 0.0100, 0.1374
    bone.roll = -0.0034
    bone.connected = True
    bone.parent = arm.edit_bones['thigh.R']
    bone = arm.edit_bones.new('spine.04')
    bone.head[:] = -0.0000, 0.0160, 1.3705
    bone.tail[:] = -0.0000, 0.0590, 1.4497
    bone.roll = -0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['spine.03']
    bone = arm.edit_bones.new('foot.L')
    bone.head[:] = 0.0915, 0.0100, 0.1374
    bone.tail[:] = 0.1033, -0.0968, 0.0510
    bone.roll = 2.8964
    bone.connected = True
    bone.parent = arm.edit_bones['shin.L']
    bone = arm.edit_bones.new('foot.R')
    bone.head[:] = -0.0915, 0.0100, 0.1374
    bone.tail[:] = -0.1033, -0.0968, 0.0510
    bone.roll = -2.8793
    bone.connected = True
    bone.parent = arm.edit_bones['shin.R']
    bone = arm.edit_bones.new('neck_base')
    bone.head[:] = -0.0000, 0.0590, 1.4497
    bone.tail[:] = -0.0000, 0.0401, 1.5389
    bone.roll = -0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['spine.04']
    bone = arm.edit_bones.new('toe.L')
    bone.head[:] = 0.1033, -0.0968, 0.0510
    bone.tail[:] = 0.1136, -0.1848, 0.0510
    bone.roll = 0.0001
    bone.connected = True
    bone.parent = arm.edit_bones['foot.L']
    bone = arm.edit_bones.new('heel.L')
    bone.head[:] = 0.0809, 0.0969, -0.0000
    bone.tail[:] = 0.1020, -0.0846, -0.0000
    bone.roll = -0.0001
    bone.connected = False
    bone.parent = arm.edit_bones['foot.L']
    bone = arm.edit_bones.new('toe.R')
    bone.head[:] = -0.1033, -0.0968, 0.0510
    bone.tail[:] = -0.1136, -0.1848, 0.0510
    bone.roll = -0.0002
    bone.connected = True
    bone.parent = arm.edit_bones['foot.R']
    bone = arm.edit_bones.new('heel.R')
    bone.head[:] = -0.0809, 0.0969, -0.0000
    bone.tail[:] = -0.1020, -0.0846, -0.0000
    bone.roll = -0.0000
    bone.connected = False
    bone.parent = arm.edit_bones['foot.R']
    bone = arm.edit_bones.new('head')
    bone.head[:] = -0.0000, 0.0401, 1.5389
    bone.tail[:] = -0.0000, 0.0401, 1.5979
    bone.roll = 3.1416
    bone.connected = True
    bone.parent = arm.edit_bones['neck_base']
    bone = arm.edit_bones.new('DLT-shoulder.L')
    bone.head[:] = 0.0141, -0.0346, 1.4991
    bone.tail[:] = 0.1226, 0.0054, 1.4991
    bone.roll = 0.0005
    bone.connected = False
    bone.parent = arm.edit_bones['neck_base']
    bone = arm.edit_bones.new('DLT-shoulder.R')
    bone.head[:] = -0.0141, -0.0346, 1.4991
    bone.tail[:] = -0.1226, 0.0054, 1.4991
    bone.roll = -0.0005
    bone.connected = False
    bone.parent = arm.edit_bones['neck_base']
    bone = arm.edit_bones.new('neck.01')
    bone.head[:] = -0.0000, 0.0401, 1.5389
    bone.tail[:] = -0.0000, 0.0176, 1.5916
    bone.roll = 0.0000
    bone.connected = False
    bone.parent = arm.edit_bones['head']
    bone = arm.edit_bones.new('shoulder.L')
    bone.head[:] = 0.0141, -0.0346, 1.4991
    bone.tail[:] = 0.1226, 0.0216, 1.5270
    bone.roll = -0.1225
    bone.connected = False
    bone.parent = arm.edit_bones['DLT-shoulder.L']
    bone = arm.edit_bones.new('shoulder.R')
    bone.head[:] = -0.0141, -0.0346, 1.4991
    bone.tail[:] = -0.1226, 0.0216, 1.5270
    bone.roll = 0.0849
    bone.connected = False
    bone.parent = arm.edit_bones['DLT-shoulder.R']
    bone = arm.edit_bones.new('neck.02')
    bone.head[:] = -0.0000, 0.0176, 1.5916
    bone.tail[:] = -0.0000, 0.0001, 1.6499
    bone.roll = 0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['neck.01']
    bone = arm.edit_bones.new('DLT-upper_arm.L')
    bone.head[:] = 0.1482, 0.0483, 1.4943
    bone.tail[:] = 0.2586, 0.1057, 1.5124
    bone.roll = 1.4969
    bone.connected = False
    bone.parent = arm.edit_bones['shoulder.L']
    bone = arm.edit_bones.new('DLT-upper_arm.R')
    bone.head[:] = -0.1482, 0.0483, 1.4943
    bone.tail[:] = -0.2586, 0.1057, 1.5124
    bone.roll = -1.4482
    bone.connected = False
    bone.parent = arm.edit_bones['shoulder.R']
    bone = arm.edit_bones.new('neck.03')
    bone.head[:] = -0.0000, 0.0001, 1.6499
    bone.tail[:] = -0.0000, 0.0001, 1.8522
    bone.roll = 0.0000
    bone.connected = True
    bone.parent = arm.edit_bones['neck.02']
    bone = arm.edit_bones.new('upper_arm.L')
    bone.head[:] = 0.1482, 0.0483, 1.4943
    bone.tail[:] = 0.3929, 0.0522, 1.4801
    bone.roll = 1.6281
    bone.connected = False
    bone.parent = arm.edit_bones['DLT-upper_arm.L']
    bone = arm.edit_bones.new('upper_arm.R')
    bone.head[:] = -0.1482, 0.0483, 1.4943
    bone.tail[:] = -0.3929, 0.0522, 1.4801
    bone.roll = -1.6281
    bone.connected = False
    bone.parent = arm.edit_bones['DLT-upper_arm.R']
    bone = arm.edit_bones.new('forearm.L')
    bone.head[:] = 0.3929, 0.0522, 1.4801
    bone.tail[:] = 0.6198, 0.0364, 1.4906
    bone.roll = 1.5240
    bone.connected = True
    bone.parent = arm.edit_bones['upper_arm.L']
    bone = arm.edit_bones.new('forearm.R')
    bone.head[:] = -0.3929, 0.0522, 1.4801
    bone.tail[:] = -0.6198, 0.0364, 1.4906
    bone.roll = -1.5219
    bone.connected = True
    bone.parent = arm.edit_bones['upper_arm.R']
    bone = arm.edit_bones.new('hand.L')
    bone.head[:] = 0.6198, 0.0364, 1.4906
    bone.tail[:] = 0.6592, 0.0364, 1.4853
    bone.roll = -3.0065
    bone.connected = True
    bone.parent = arm.edit_bones['forearm.L']
    bone = arm.edit_bones.new('hand.R')
    bone.head[:] = -0.6198, 0.0364, 1.4906
    bone.tail[:] = -0.6592, 0.0364, 1.4853
    bone.roll = 3.0065
    bone.connected = True
    bone.parent = arm.edit_bones['forearm.R']
    bone = arm.edit_bones.new('palm.04.L')
    bone.head[:] = 0.6514, 0.0658, 1.4906
    bone.tail[:] = 0.7287, 0.0810, 1.4747
    bone.roll = -3.0715
    bone.connected = False
    bone.parent = arm.edit_bones['hand.L']
    bone = arm.edit_bones.new('palm.03.L')
    bone.head[:] = 0.6533, 0.0481, 1.4943
    bone.tail[:] = 0.7386, 0.0553, 1.4781
    bone.roll = -3.0290
    bone.connected = False
    bone.parent = arm.edit_bones['hand.L']
    bone = arm.edit_bones.new('palm.02.L')
    bone.head[:] = 0.6539, 0.0305, 1.4967
    bone.tail[:] = 0.7420, 0.0250, 1.4835
    bone.roll = -3.0669
    bone.connected = False
    bone.parent = arm.edit_bones['hand.L']
    bone = arm.edit_bones.new('palm.01.L')
    bone.head[:] = 0.6514, 0.0116, 1.4961
    bone.tail[:] = 0.7361, -0.0074, 1.4823
    bone.roll = -2.9422
    bone.connected = False
    bone.parent = arm.edit_bones['hand.L']
    bone = arm.edit_bones.new('thumb.01.L')
    bone.head[:] = 0.6380, -0.0005, 1.4848
    bone.tail[:] = 0.6757, -0.0408, 1.4538
    bone.roll = -0.7041
    bone.connected = False
    bone.parent = arm.edit_bones['hand.L']
    bone = arm.edit_bones.new('palm.04.R')
    bone.head[:] = -0.6514, 0.0658, 1.4906
    bone.tail[:] = -0.7287, 0.0810, 1.4747
    bone.roll = 3.0715
    bone.connected = False
    bone.parent = arm.edit_bones['hand.R']
    bone = arm.edit_bones.new('palm.03.R')
    bone.head[:] = -0.6533, 0.0481, 1.4943
    bone.tail[:] = -0.7386, 0.0553, 1.4781
    bone.roll = 3.0290
    bone.connected = False
    bone.parent = arm.edit_bones['hand.R']
    bone = arm.edit_bones.new('palm.02.R')
    bone.head[:] = -0.6539, 0.0305, 1.4967
    bone.tail[:] = -0.7420, 0.0250, 1.4835
    bone.roll = 3.0669
    bone.connected = False
    bone.parent = arm.edit_bones['hand.R']
    bone = arm.edit_bones.new('thumb.01.R')
    bone.head[:] = -0.6380, -0.0005, 1.4848
    bone.tail[:] = -0.6757, -0.0408, 1.4538
    bone.roll = 0.7041
    bone.connected = False
    bone.parent = arm.edit_bones['hand.R']
    bone = arm.edit_bones.new('palm.01.R')
    bone.head[:] = -0.6514, 0.0116, 1.4961
    bone.tail[:] = -0.7361, -0.0074, 1.4823
    bone.roll = 2.9332
    bone.connected = False
    bone.parent = arm.edit_bones['hand.R']
    bone = arm.edit_bones.new('finger_pinky.01.L')
    bone.head[:] = 0.7287, 0.0810, 1.4747
    bone.tail[:] = 0.7698, 0.0947, 1.4635
    bone.roll = -3.0949
    bone.connected = True
    bone.parent = arm.edit_bones['palm.04.L']
    bone = arm.edit_bones.new('finger_ring.01.L')
    bone.head[:] = 0.7386, 0.0553, 1.4781
    bone.tail[:] = 0.7890, 0.0615, 1.4667
    bone.roll = -3.0081
    bone.connected = True
    bone.parent = arm.edit_bones['palm.03.L']
    bone = arm.edit_bones.new('finger_middle.01.L')
    bone.head[:] = 0.7420, 0.0250, 1.4835
    bone.tail[:] = 0.7975, 0.0221, 1.4712
    bone.roll = -2.9982
    bone.connected = True
    bone.parent = arm.edit_bones['palm.02.L']
    bone = arm.edit_bones.new('finger_index.01.L')
    bone.head[:] = 0.7361, -0.0074, 1.4823
    bone.tail[:] = 0.7843, -0.0204, 1.4718
    bone.roll = -3.0021
    bone.connected = True
    bone.parent = arm.edit_bones['palm.01.L']
    bone = arm.edit_bones.new('thumb.02.L')
    bone.head[:] = 0.6757, -0.0408, 1.4538
    bone.tail[:] = 0.6958, -0.0568, 1.4376
    bone.roll = -0.6963
    bone.connected = True
    bone.parent = arm.edit_bones['thumb.01.L']
    bone = arm.edit_bones.new('finger_pinky.01.R')
    bone.head[:] = -0.7287, 0.0810, 1.4747
    bone.tail[:] = -0.7698, 0.0947, 1.4635
    bone.roll = 3.0949
    bone.connected = True
    bone.parent = arm.edit_bones['palm.04.R']
    bone = arm.edit_bones.new('finger_ring.01.R')
    bone.head[:] = -0.7386, 0.0553, 1.4781
    bone.tail[:] = -0.7890, 0.0615, 1.4667
    bone.roll = 2.9892
    bone.connected = True
    bone.parent = arm.edit_bones['palm.03.R']
    bone = arm.edit_bones.new('finger_middle.01.R')
    bone.head[:] = -0.7420, 0.0250, 1.4835
    bone.tail[:] = -0.7975, 0.0221, 1.4712
    bone.roll = 2.9816
    bone.connected = True
    bone.parent = arm.edit_bones['palm.02.R']
    bone = arm.edit_bones.new('thumb.02.R')
    bone.head[:] = -0.6757, -0.0408, 1.4538
    bone.tail[:] = -0.6958, -0.0568, 1.4376
    bone.roll = 0.6963
    bone.connected = True
    bone.parent = arm.edit_bones['thumb.01.R']
    bone = arm.edit_bones.new('finger_index.01.R')
    bone.head[:] = -0.7361, -0.0074, 1.4823
    bone.tail[:] = -0.7843, -0.0204, 1.4718
    bone.roll = 2.9498
    bone.connected = True
    bone.parent = arm.edit_bones['palm.01.R']
    bone = arm.edit_bones.new('finger_pinky.02.L')
    bone.head[:] = 0.7698, 0.0947, 1.4635
    bone.tail[:] = 0.7910, 0.1018, 1.4577
    bone.roll = -3.0949
    bone.connected = True
    bone.parent = arm.edit_bones['finger_pinky.01.L']
    bone = arm.edit_bones.new('finger_ring.02.L')
    bone.head[:] = 0.7890, 0.0615, 1.4667
    bone.tail[:] = 0.8177, 0.0650, 1.4600
    bone.roll = -3.0006
    bone.connected = True
    bone.parent = arm.edit_bones['finger_ring.01.L']
    bone = arm.edit_bones.new('finger_middle.02.L')
    bone.head[:] = 0.7975, 0.0221, 1.4712
    bone.tail[:] = 0.8289, 0.0206, 1.4643
    bone.roll = -2.9995
    bone.connected = True
    bone.parent = arm.edit_bones['finger_middle.01.L']
    bone = arm.edit_bones.new('finger_index.02.L')
    bone.head[:] = 0.7843, -0.0204, 1.4718
    bone.tail[:] = 0.8117, -0.0275, 1.4660
    bone.roll = -3.0064
    bone.connected = True
    bone.parent = arm.edit_bones['finger_index.01.L']
    bone = arm.edit_bones.new('thumb.03.L')
    bone.head[:] = 0.6958, -0.0568, 1.4376
    bone.tail[:] = 0.7196, -0.0671, 1.4210
    bone.roll = -0.8072
    bone.connected = True
    bone.parent = arm.edit_bones['thumb.02.L']
    bone = arm.edit_bones.new('finger_pinky.02.R')
    bone.head[:] = -0.7698, 0.0947, 1.4635
    bone.tail[:] = -0.7910, 0.1018, 1.4577
    bone.roll = 3.0949
    bone.connected = True
    bone.parent = arm.edit_bones['finger_pinky.01.R']
    bone = arm.edit_bones.new('finger_ring.02.R')
    bone.head[:] = -0.7890, 0.0615, 1.4667
    bone.tail[:] = -0.8177, 0.0650, 1.4600
    bone.roll = 3.0341
    bone.connected = True
    bone.parent = arm.edit_bones['finger_ring.01.R']
    bone = arm.edit_bones.new('finger_middle.02.R')
    bone.head[:] = -0.7975, 0.0221, 1.4712
    bone.tail[:] = -0.8289, 0.0206, 1.4643
    bone.roll = 3.0291
    bone.connected = True
    bone.parent = arm.edit_bones['finger_middle.01.R']
    bone = arm.edit_bones.new('thumb.03.R')
    bone.head[:] = -0.6958, -0.0568, 1.4376
    bone.tail[:] = -0.7196, -0.0671, 1.4210
    bone.roll = 0.8072
    bone.connected = True
    bone.parent = arm.edit_bones['thumb.02.R']
    bone = arm.edit_bones.new('finger_index.02.R')
    bone.head[:] = -0.7843, -0.0204, 1.4718
    bone.tail[:] = -0.8117, -0.0275, 1.4660
    bone.roll = 3.0705
    bone.connected = True
    bone.parent = arm.edit_bones['finger_index.01.R']
    bone = arm.edit_bones.new('finger_pinky.03.L')
    bone.head[:] = 0.7910, 0.1018, 1.4577
    bone.tail[:] = 0.8109, 0.1085, 1.4523
    bone.roll = -3.0949
    bone.connected = True
    bone.parent = arm.edit_bones['finger_pinky.02.L']
    bone = arm.edit_bones.new('finger_ring.03.L')
    bone.head[:] = 0.8177, 0.0650, 1.4600
    bone.tail[:] = 0.8396, 0.0677, 1.4544
    bone.roll = -2.9819
    bone.connected = True
    bone.parent = arm.edit_bones['finger_ring.02.L']
    bone = arm.edit_bones.new('finger_middle.03.L')
    bone.head[:] = 0.8289, 0.0206, 1.4643
    bone.tail[:] = 0.8534, 0.0193, 1.4589
    bone.roll = -3.0004
    bone.connected = True
    bone.parent = arm.edit_bones['finger_middle.02.L']
    bone = arm.edit_bones.new('finger_index.03.L')
    bone.head[:] = 0.8117, -0.0275, 1.4660
    bone.tail[:] = 0.8331, -0.0333, 1.4615
    bone.roll = -3.0103
    bone.connected = True
    bone.parent = arm.edit_bones['finger_index.02.L']
    bone = arm.edit_bones.new('finger_pinky.03.R')
    bone.head[:] = -0.7910, 0.1018, 1.4577
    bone.tail[:] = -0.8109, 0.1085, 1.4523
    bone.roll = 3.0949
    bone.connected = True
    bone.parent = arm.edit_bones['finger_pinky.02.R']
    bone = arm.edit_bones.new('finger_ring.03.R')
    bone.head[:] = -0.8177, 0.0650, 1.4600
    bone.tail[:] = -0.8396, 0.0677, 1.4544
    bone.roll = 2.9819
    bone.connected = True
    bone.parent = arm.edit_bones['finger_ring.02.R']
    bone = arm.edit_bones.new('finger_middle.03.R')
    bone.head[:] = -0.8289, 0.0206, 1.4643
    bone.tail[:] = -0.8534, 0.0193, 1.4589
    bone.roll = 3.0004
    bone.connected = True
    bone.parent = arm.edit_bones['finger_middle.02.R']
    bone = arm.edit_bones.new('finger_index.03.R')
    bone.head[:] = -0.8117, -0.0275, 1.4660
    bone.tail[:] = -0.8331, -0.0333, 1.4615
    bone.roll = 2.9917
    bone.connected = True
    bone.parent = arm.edit_bones['finger_index.02.R']

    bpy.ops.object.mode_set(mode='OBJECT')
    pbone = obj.pose.bones['root']
    pbone['type'] = 'root'
    pbone = obj.pose.bones['root']
    pbone['root.layer'] = 16
    pbone = obj.pose.bones['torso']
    pbone['type'] = 'spine_pivot_flex'
    pbone = obj.pose.bones['torso']
    pbone['spine_pivot_flex.later_main'] = 1
    pbone = obj.pose.bones['torso']
    pbone['spine_pivot_flex.layer_extra'] = 2
    pbone = obj.pose.bones['thigh.L']
    pbone['type'] = 'leg_biped_generic'
    pbone = obj.pose.bones['thigh.L']
    pbone['leg_biped_generic.layer_ik'] = 12
    pbone = obj.pose.bones['thigh.L']
    pbone['leg_biped_generic.layer_fk'] = 11
    pbone = obj.pose.bones['thigh.R']
    pbone['type'] = 'leg_biped_generic'
    pbone = obj.pose.bones['thigh.R']
    pbone['leg_biped_generic.layer_ik'] = 14
    pbone = obj.pose.bones['thigh.R']
    pbone['leg_biped_generic.layer_fk'] = 13
    pbone = obj.pose.bones['head']
    pbone['type'] = 'neck_flex'
    pbone = obj.pose.bones['head']
    pbone['neck_flex.layer_extra'] = 4
    pbone = obj.pose.bones['head']
    pbone['neck_flex.layer_main'] = 3
    pbone = obj.pose.bones['DLT-shoulder.L']
    pbone['type'] = 'delta'
    pbone = obj.pose.bones['DLT-shoulder.R']
    pbone['type'] = 'delta'
    pbone = obj.pose.bones['shoulder.L']
    pbone['type'] = 'copy'
    pbone = obj.pose.bones['shoulder.L']
    pbone['copy.layer'] = 1
    pbone = obj.pose.bones['shoulder.R']
    pbone['type'] = 'copy'
    pbone = obj.pose.bones['shoulder.R']
    pbone['copy.layer'] = 1
    pbone = obj.pose.bones['DLT-upper_arm.L']
    pbone['type'] = 'delta'
    pbone = obj.pose.bones['DLT-upper_arm.R']
    pbone['type'] = 'delta'
    pbone = obj.pose.bones['upper_arm.L']
    pbone['type'] = 'arm_biped_generic'
    pbone = obj.pose.bones['upper_arm.L']
    pbone['arm_biped_generic.elbow_parent'] = 'spine.04'
    pbone = obj.pose.bones['upper_arm.L']
    pbone['arm_biped_generic.layer_fk'] = 7
    pbone = obj.pose.bones['upper_arm.L']
    pbone['arm_biped_generic.layer_ik'] = 8
    pbone = obj.pose.bones['upper_arm.R']
    pbone['type'] = 'arm_biped_generic'
    pbone = obj.pose.bones['upper_arm.R']
    pbone['arm_biped_generic.layer_fk'] = 9
    pbone = obj.pose.bones['upper_arm.R']
    pbone['arm_biped_generic.layer_ik'] = 10
    pbone = obj.pose.bones['upper_arm.R']
    pbone['arm_biped_generic.elbow_parent'] = 'spine.04'
    pbone = obj.pose.bones['palm.01.L']
    pbone['type'] = 'palm_curl'
    pbone = obj.pose.bones['palm.01.L']
    pbone['palm_curl.layer'] = 5
    pbone = obj.pose.bones['thumb.01.L']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['thumb.01.L']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['thumb.01.L']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['thumb.01.R']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['thumb.01.R']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['thumb.01.R']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['palm.01.R']
    pbone['type'] = 'palm_curl'
    pbone = obj.pose.bones['palm.01.R']
    pbone['palm_curl.layer'] = 5
    pbone = obj.pose.bones['finger_pinky.01.L']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_pinky.01.L']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_pinky.01.L']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_ring.01.L']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_ring.01.L']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_ring.01.L']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_middle.01.L']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_middle.01.L']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_middle.01.L']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_index.01.L']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_index.01.L']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_index.01.L']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_pinky.01.R']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_pinky.01.R']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_pinky.01.R']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_ring.01.R']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_ring.01.R']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_ring.01.R']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_middle.01.R']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_middle.01.R']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_middle.01.R']
    pbone['finger_curl.layer_extra'] = 6
    pbone = obj.pose.bones['finger_index.01.R']
    pbone['type'] = 'finger_curl'
    pbone = obj.pose.bones['finger_index.01.R']
    pbone['finger_curl.layer_main'] = 5
    pbone = obj.pose.bones['finger_index.01.R']
    pbone['finger_curl.layer_extra'] = 6


class AddHuman(bpy.types.Operator):
    '''Add an advanced human metarig base.'''
    bl_idname = "object.armature_human_advanced_add"
    bl_label = "Add Humanoid (advanced metarig)"
    bl_register = True
    bl_undo = True

    def execute(self, context):
        bpy.ops.object.armature_add()
        obj = context.active_object
        mode_orig = obj.mode
        bpy.ops.object.mode_set(mode='EDIT') # grr, remove bone
        bones = context.active_object.data.edit_bones
        bones.remove(bones[0])
        metarig_template()
        bpy.ops.object.mode_set(mode=mode_orig)
        return {'FINISHED'}

# Register the operator
bpy.types.register(AddHuman)

# Add to a menu
import dynamic_menu

menu_func = (lambda self, context: self.layout.operator(AddHuman.bl_idname, icon='OUTLINER_OB_ARMATURE', text="Human (Meta-Rig)"))

menu_item = dynamic_menu.add(bpy.types.INFO_MT_armature_add, menu_func)

if __name__ == "__main__":
    bpy.ops.mesh.armature_human_advanced_add()
