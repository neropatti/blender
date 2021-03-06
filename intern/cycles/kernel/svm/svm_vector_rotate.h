/*
 * Copyright 2011-2020 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

CCL_NAMESPACE_BEGIN

/* Vector Rotate */

ccl_device void svm_node_vector_rotate(ShaderData *sd,
                                       float *stack,
                                       uint input_stack_offsets,
                                       uint axis_stack_offsets,
                                       uint result_stack_offset)
{
  uint type, vector_stack_offset, rotation_stack_offset, center_stack_offset, axis_stack_offset,
      angle_stack_offset;

  svm_unpack_node_uchar3(input_stack_offsets, &type, &vector_stack_offset, &rotation_stack_offset);
  svm_unpack_node_uchar3(
      axis_stack_offsets, &center_stack_offset, &axis_stack_offset, &angle_stack_offset);

  float3 vector = stack_load_float3(stack, vector_stack_offset);
  float3 center = stack_load_float3(stack, center_stack_offset);
  float3 result = make_float3(0.0f, 0.0f, 0.0f);

  if (type != NODE_VECTOR_ROTATE_TYPE_AXIS && type != NODE_VECTOR_ROTATE_TYPE_AXIS_X &&
      type != NODE_VECTOR_ROTATE_TYPE_AXIS_Y && type != NODE_VECTOR_ROTATE_TYPE_AXIS_Z) {
    float3 rotation = stack_load_float3(stack, rotation_stack_offset);  // Default XYZ.
    switch (type) {
      case NODE_VECTOR_ROTATE_TYPE_EULER_XZY:
        rotation = make_float3(-rotation.x, -rotation.z, -rotation.y);
        break;
      case NODE_VECTOR_ROTATE_TYPE_EULER_YXZ:
        rotation = make_float3(-rotation.y, -rotation.x, -rotation.z);
        break;
      case NODE_VECTOR_ROTATE_TYPE_EULER_YZX:
        rotation = make_float3(rotation.y, rotation.z, rotation.x);
        break;
      case NODE_VECTOR_ROTATE_TYPE_EULER_ZXY:
        rotation = make_float3(rotation.z, rotation.x, rotation.y);
        break;
      case NODE_VECTOR_ROTATE_TYPE_EULER_ZYX:
        rotation = make_float3(-rotation.z, -rotation.y, -rotation.x);
        break;
    }
    Transform rotationTransform = euler_to_transform(rotation);
    result = transform_direction(&rotationTransform, vector - center) + center;
  }
  else {
    float3 axis;
    switch (type) {
      case NODE_VECTOR_ROTATE_TYPE_AXIS_X:
        axis = make_float3(1.0f, 0.0f, 0.0f);
        break;
      case NODE_VECTOR_ROTATE_TYPE_AXIS_Y:
        axis = make_float3(0.0f, 1.0f, 0.0f);
        break;
      case NODE_VECTOR_ROTATE_TYPE_AXIS_Z:
        axis = make_float3(0.0f, 0.0f, 1.0f);
        break;
      default:
        axis = normalize(stack_load_float3(stack, axis_stack_offset));
        break;
    }
    float angle = stack_load_float(stack, angle_stack_offset);
    result = is_zero(axis) ? vector : rotate_around_axis(vector - center, axis, angle) + center;
  }

  /* Output */
  if (stack_valid(result_stack_offset)) {
    stack_store_float3(stack, result_stack_offset, result);
  }
}

CCL_NAMESPACE_END
