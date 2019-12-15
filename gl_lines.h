#ifndef GL_LINES_H
#define GL_LINES_H

void* gl_lines_init_device( void );
uint32_t gl_lines_update( void* device, const void* data, int32_t n_elems, int32_t elem_size );
void gl_lines_render( const void* device, const int32_t count, const float* mvp );

//TODO(maciej): Terminate device!

#endif /* GL_LINES_H */

#ifdef GL_LINES_IMPLEMENTATION

typedef struct gl_lines_device
{
  GLuint program_id;
  GLuint uniform_mvp_location;
  GLuint attrib_pos_location;
  GLuint attrib_col_location;
  GLuint vao;
  GLuint vbo;
} gl_lines_device_t;


void
gl_lines_assert_shader_compiled( GLuint shader_id )
{
  GLint status;
  glGetShaderiv( shader_id, GL_COMPILE_STATUS, &status );
  if( status == GL_FALSE )
  {
    int32_t info_len = 0;
    glGetShaderiv( shader_id, GL_INFO_LOG_LENGTH, &info_len );
    GLchar* info = malloc( info_len );
    glGetShaderInfoLog( shader_id, info_len, &info_len, info );
    fprintf( stderr, "[GL] Compile error: \n%s\n", info );
    free( info );
    exit( -1 );
  }
}

void
gl_lines_assert_program_linked( GLuint program_id )
{
  GLint status;
  glGetProgramiv( program_id, GL_LINK_STATUS, &status );
  if( status == GL_FALSE )
  {
    int32_t info_len = 0;
    glGetProgramiv( program_id, GL_INFO_LOG_LENGTH, &info_len );
    GLchar* info = malloc( info_len );
    glGetProgramInfoLog( program_id, info_len, &info_len, info );
    fprintf( stderr, "[GL] Link error: \n%s\n", info );
    free( info );
    exit(-1);
  }
}

void
gl_lines_create_shader_program( gl_lines_device_t* device )
{
  const char* vs_src = 
    SHDR_VERSION
    SHDR_SOURCE(
      layout(location = 0) in vec3 pos;
      layout(location = 1) in vec3 col;
      
      layout(location = 0) uniform mat4 u_mvp;

      out vec3 v_col;

      void main()
      {
        v_col = col;
        gl_Position = u_mvp * vec4(pos, 1.0);
      }
    );
  
  const char* fs_src = 
    SHDR_VERSION
    SHDR_SOURCE(
      in vec3 v_col;
      out vec4 frag_color;
      void main()
      {
        frag_color = vec4(v_col, 1.0);
      }
    );

  GLuint vertex_shader   = glCreateShader( GL_VERTEX_SHADER );
  GLuint fragment_shader = glCreateShader( GL_FRAGMENT_SHADER );

  glShaderSource( vertex_shader, 1, &vs_src, 0 );
  glCompileShader( vertex_shader );
  gl_lines_assert_shader_compiled( vertex_shader );

  glShaderSource( fragment_shader, 1, &fs_src, 0 );
  glCompileShader( fragment_shader );
  gl_lines_assert_shader_compiled( fragment_shader );


  device->program_id = glCreateProgram();
  glAttachShader( device->program_id, vertex_shader );
  glAttachShader( device->program_id, fragment_shader );
  glLinkProgram( device->program_id );
  gl_lines_assert_program_linked( device->program_id );

  glDetachShader( device->program_id, vertex_shader );
  glDetachShader( device->program_id, fragment_shader );
  glDeleteShader( vertex_shader );
  glDeleteShader( fragment_shader );

  device->attrib_pos_location = glGetAttribLocation( device->program_id, "pos" );
  device->attrib_col_location = glGetAttribLocation( device->program_id, "col" );
  device->uniform_mvp_location = glGetUniformLocation( device->program_id, "u_mvp" );
}

void
gl_lines_setup_geometry_storage( gl_lines_device_t* device )
{
  GLuint  stream_idx = 0;
  glCreateVertexArrays( 1, &device->vao );
  glCreateBuffers( 1, &device->vbo );
  glNamedBufferStorage( device->vbo, MAX_VERTS * sizeof(vertex_t), NULL, GL_DYNAMIC_STORAGE_BIT );

  glVertexArrayVertexBuffer( device->vao, stream_idx, device->vbo, 0, sizeof(vertex_t) );

  glEnableVertexArrayAttrib( device->vao, device->attrib_pos_location );
  glEnableVertexArrayAttrib( device->vao, device->attrib_col_location );

  glVertexArrayAttribFormat( device->vao, device->attrib_pos_location, 3, GL_FLOAT, GL_FALSE, offsetof(vertex_t, pos) );
  glVertexArrayAttribFormat( device->vao, device->attrib_col_location, 3, GL_FLOAT, GL_FALSE, offsetof(vertex_t, col) );

  glVertexArrayAttribBinding( device->vao, device->attrib_pos_location, stream_idx );
  glVertexArrayAttribBinding( device->vao, device->attrib_col_location, stream_idx );
}

void*
gl_lines_init_device( void )
{
  gl_lines_device_t* device = malloc( sizeof(gl_lines_device_t) );
  memset( device, 0, sizeof(gl_lines_device_t) );
  gl_lines_create_shader_program( device );
  gl_lines_setup_geometry_storage( device );
  return device;
}

uint32_t
gl_lines_update( void* device_in, const void* data, int32_t n_elems, int32_t elem_size )
{
  gl_lines_device_t* device = device_in;
  glNamedBufferSubData( device->vbo, 0, n_elems*elem_size, data );
  return n_elems;
}

void
gl_lines_render( const void* device_in, const int32_t count, const float* mvp )
{
  const gl_lines_device_t* device = device_in;
  glUseProgram( device->program_id );
  glUniformMatrix4fv( device->uniform_mvp_location, 1, GL_FALSE, mvp );

  glBindVertexArray( device->vao );
  glDrawArrays( GL_LINES, 0, count );

  glBindVertexArray( 0 );
  glUseProgram( 0 );
}

#endif /*GL_LINES_IMPLEMENTATION*/