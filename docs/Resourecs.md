# General
## Text resources
- All text resources must have a tag identifying the resource type.
- Resource arguments are separated by ':' characters ','
    - I.e. the line "name:example_name" would result in the args being:
        1. name
        2. example_name
```#resource [type] (Ex. #resource shader)```
<!-- - All resources are converted to binary resources by the build system. -->

# Shaders
## Variables
- name
- stage
    - Values: vert, vertex, frag, fragment
- vertex_attributes
    - The layout of the vertex attributes must be defined in the order of their layout.
        - I.e. A shader with the following layout
            ```glsl
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec3 in_normal;
            layout(location = 2) in vec2 uv;
            ```
          would be defined as
            ```
            vertex_attributes:position,normal,uv0
            ```
    - Values: positon, normal, uv0
- resource
    - Format: resource:[type],[stage],[binding],[set]
        - Type: uniform_buffer, sampler
        - Stage: vert, vertex, frag, fragment
        - Binding: The binding of the resource
        - Set: The resource set
    - Example:
        Given a resource in the vertex shader,

        ```glsl
        layout(set = 0, binding = 1) uniform instance_data { mat4 view_matrix; } instance;

        ```
        the matching resource would be

        ```
        resource:unofrom_buffer,vert,0,1
        ```
## Example
```
    #resource shader
    name:example
    stage:frag,vert
    vertex_attributes:position,normal,uv0
    resource:uniform_buffer,vert,0,0
    resource:sampler,frag,0,1
    ```


