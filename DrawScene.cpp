//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <FreeImage/FreeImage.h>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS, h_ShaderProgram_GR, h_ShaderProgram_BL; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4
GLint loc_global_ambient_color;
GLint loc_global_ambient_color_GR;
GLint loc_global_ambient_color_BL;

loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_light_Parameters loc_light_GR[NUMBER_OF_LIGHT_SUPPORTED];
loc_light_Parameters loc_light_BL[NUMBER_OF_LIGHT_SUPPORTED];

GLint loc_flag_diffuse_texture_mapping, loc_flag_normal_texture_mapping, loc_flag_emissive_texture_mapping, loc_flag_normal_based_directX;
GLint loc_flag_diffuse_texture_mapping_GR, loc_flag_normal_texture_mapping_GR, loc_flag_emissive_texture_mapping_GR, loc_flag_normal_based_directX_GR;
GLint loc_flag_diffuse_texture_mapping_BL, loc_flag_normal_texture_mapping_BL, loc_flag_emissive_texture_mapping_BL, loc_flag_normal_based_directX_BL;
GLint loc_u_flag_blending;
GLint loc_u_fragment_alpha;
GLint loc_all_directions;

loc_Material_Parameters loc_material;
loc_Material_Parameters loc_material_GR;
loc_Material_Parameters loc_material_BL;

GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_ModelViewProjectionMatrix_GR, loc_ModelViewMatrix_GR, loc_ModelViewMatrixInvTrans_GR;
GLint loc_ModelViewProjectionMatrix_BL, loc_ModelViewMatrix_BL, loc_ModelViewMatrixInvTrans_BL;

GLint loc_flag_fog;
GLint loc_flag_fog_GR;
GLint loc_flag_fog_BL;
int flag_fog;
GLint loc_diffuse_texture;
int flag_blend_mode = 0;
int all_directions = 0;

// for skybox shaders
GLuint h_ShaderProgram_skybox;
GLint loc_cubemap_skybox;
GLint loc_ModelViewProjectionMatrix_SKY;


#define TEXTURE_FLOOR				(0)
#define TEXTURE_TIGER				(1)
#define TEXTURE_WOLF				(2)
#define TEXTURE_SPIDER				(3)
#define TEXTURE_DRAGON				(4)
#define TEXTURE_OPTIMUS				(5)
#define TEXTURE_COW					(6)
#define TEXTURE_BUS					(7)
#define TEXTURE_BIKE				(8)
#define TEXTURE_GODZILLA			(9)
#define TEXTURE_IRONMAN				(10)
#define TEXTURE_TANK				(11)
#define TEXTURE_NATHAN				(12)
#define TEXTURE_OGRE				(13)
#define TEXTURE_CAT					(14)
#define TEXTURE_ANT					(15)
#define TEXTURE_TOWER				(16)
#define N_TEXTURES_USED				(17)

GLuint texture_names[N_TEXTURES_USED];

#define TEXTURE_INDEX_DIFFUSE	(0)
#define TEXTURE_INDEX_NORMAL	(1)
#define TEXTURE_INDEX_SPECULAR	(2)
#define TEXTURE_INDEX_EMISSIVE	(3)
#define TEXTURE_INDEX_SKYMAP	(4)


// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> 
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix, WolfModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans, WolfModelViewMatrixInvTrans;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define MOVE_SPEED 100
#define CAM_RSPEED 0.1f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define WOLF_ROTATION_RADIUS 1500

#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2

unsigned int _timestamp_scene = 0; // the global clock in the scene
int cur_frame_tiger = 0, cur_frame_ben = 0, cur_frame_wolf, cur_frame_spider = 0, cur_frame_nathan = 0;
int ctrl_pressed = 0, shift_pressed = 0, leftbuttonpressed = 0, rightbuttonpressed = 0;
float ironmanScale = 75.0;
float dragonScale = 140.0;
int cube_degree = 0;
int cube_dx = 1;
float cube_alpha = 0.4f;
float not_cube_alpha = 1.0;
int gouraud = 0, phong = 1;
int blendFlag = 0;

/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_1,
	CAMERA_2,
	CAMERA_3,
	CAMERA_4,
	CAMERA_5,
	CAMERA_6,
	CAMERA_u,
	CAMERA_i,
	CAMERA_o,
	CAMERA_p,
	CAMERA_a,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;



using glm::mat4;
void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
}

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];

	memcpy(&current_camera, pCamera, sizeof(Camera));
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {
	//CAMERA_1 : original view
	Camera* pCamera = &camera_info[CAMERA_1];
	for (int k = 0; k < 3; k++)
	{
		pCamera->pos[k] = scene.camera.e[k];
		pCamera->uaxis[k] = scene.camera.u[k]; //right
		pCamera->vaxis[k] = scene.camera.v[k]; //up
		pCamera->naxis[k] = scene.camera.n[k]; //back
	}

	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f; //setting projection mat (field of view)

	//CAMERA_2 : bistro view
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = -241.351807f; pCamera->pos[1] = 1444.346313f; pCamera->pos[2] = 283.968231f;
	pCamera->uaxis[0] = -0.998411f; pCamera->uaxis[1] = 0.049119f; pCamera->uaxis[2] = -0.027553f;
	pCamera->vaxis[0] = -0.028485f; pCamera->vaxis[1] = -0.018375f; pCamera->vaxis[2] = 0.999417f;
	pCamera->naxis[0] = 0.048585f; pCamera->naxis[1] = 0.998617f; pCamera->naxis[2] = 0.019746f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_3 : tree view
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = 1974.856567f; pCamera->pos[1] = -1588.545776f; pCamera->pos[2] = 98.843971f;
	pCamera->uaxis[0] = -0.357811f; pCamera->uaxis[1] = -0.933725f; pCamera->uaxis[2] = 0.010082f;
	pCamera->vaxis[0] = -0.180880f; pCamera->vaxis[1] = 0.079899f; pCamera->vaxis[2] = 0.980231f;
	pCamera->naxis[0] = -0.916095f; pCamera->naxis[1] = 0.348920f; pCamera->naxis[2] = -0.197483f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_4 : top view
	pCamera = &camera_info[CAMERA_4];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 18300.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 1.0f; pCamera->vaxis[2] = 0.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 1.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_5 : front view
	pCamera = &camera_info[CAMERA_5];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 11700.0f; pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 1.0f; pCamera->naxis[2] = 0.0f; //naxis 부터 setting하면 이해하기 쉬움
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_6 : side view
	pCamera = &camera_info[CAMERA_6];
	pCamera->pos[0] = 14600.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = 0.0f; pCamera->uaxis[1] = 1.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 1.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_u : gate to tree
	pCamera = &camera_info[CAMERA_u];
	pCamera->pos[0] = -950.0; pCamera->pos[1] = -600.0f; pCamera->pos[2] = 275.0f;
	pCamera->uaxis[0] = 0.0f; pCamera->uaxis[1] = 1.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = -1.0f; pCamera->naxis[1] = 0.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_i : gallery
	pCamera = &camera_info[CAMERA_i];
	pCamera->pos[0] = -1500.0f; pCamera->pos[1] = 100.0f; pCamera->pos[2] = 230.0f;
	pCamera->uaxis[0] = 0.9009688679; pCamera->uaxis[1] = 0.4338837391; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.4338837391; pCamera->naxis[1] = -0.9009688679; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_o : store sideview
	pCamera = &camera_info[CAMERA_o];
	pCamera->pos[0] = 550.0f; pCamera->pos[1] = 2125.0f; pCamera->pos[2] = 200.0f;
	pCamera->uaxis[0] = 1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = -1.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_p : reversed tree gate
	pCamera = &camera_info[CAMERA_p];
	pCamera->pos[0] = 2250.85f; pCamera->pos[1] = -1650.545776f; pCamera->pos[2] = 400.843971f;
	pCamera->uaxis[0] = -0.357811f; pCamera->uaxis[1] = -0.933725f; pCamera->uaxis[2] = 0.010082f;
	pCamera->vaxis[0] = -0.180880f; pCamera->vaxis[1] = 0.079899f; pCamera->vaxis[2] = 0.980231f;
	pCamera->naxis[0] = 0.916095f; pCamera->naxis[1] = -0.348920f; pCamera->naxis[2] = 0.197483f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	//CAMERA_a : user
	pCamera = &camera_info[CAMERA_a];
	pCamera->pos[0] = -500; pCamera->pos[1] = 500; pCamera->pos[2] = 220;
	pCamera->uaxis[0] = 0.997452; pCamera->uaxis[1] = 0.071339; pCamera->uaxis[2] = 0;
	pCamera->vaxis[0] = 0; pCamera->vaxis[1] = 0; pCamera->vaxis[2] = 1;
	pCamera->naxis[0] = -0.071339; pCamera->naxis[1] = 0.997452; pCamera->naxis[2] = 0;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 50000.0f;

	set_current_camera(CAMERA_1);
}
/*********************************  END: camera *********************************/

void My_glTexImage2D_from_file(const char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	FreeImage_FlipVertical(tx_pixmap);

	fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);
}

//for one material
int read_geometry_vnt(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);

	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

// for multiple materials
int read_geometry_vntm(GLfloat** object, int bytes_per_primitive,
	int* n_matrial_indicies, int** material_indicies,
	int* n_materials, char*** diffuse_texture_names,
	Material_Parameters** material_parameters,
	bool* bOnce,
	char* filename) {
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}

	int n_faces;
	fread(&n_faces, sizeof(int), 1, fp);

	*object = (float*)malloc(n_faces * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_faces, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);

	fread(n_matrial_indicies, sizeof(int), 1, fp);

	int bytes_per_indices = sizeof(int) * 2;
	*material_indicies = (int*)malloc(bytes_per_indices * (*n_matrial_indicies)); // material id, offset
	if (*material_indicies == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...\n", filename);
		return -1;
	}

	fread(*material_indicies, bytes_per_indices, (*n_matrial_indicies), fp);

	if (*bOnce == false) {
		fread(n_materials, sizeof(int), 1, fp);

		*material_parameters = (Material_Parameters*)malloc(sizeof(Material_Parameters) * (*n_materials));
		*diffuse_texture_names = (char**)malloc(sizeof(char*) * (*n_materials));
		for (int i = 0; i < (*n_materials); i++) {
			fread((*material_parameters)[i].ambient_color, sizeof(float), 3, fp); //Ka
			fread((*material_parameters)[i].diffuse_color, sizeof(float), 3, fp); //Kd
			fread((*material_parameters)[i].specular_color, sizeof(float), 3, fp); //Ks
			fread(&(*material_parameters)[i].specular_exponent, sizeof(float), 1, fp); //Ns
			fread((*material_parameters)[i].emissive_color, sizeof(float), 3, fp); //Ke

			(*material_parameters)[i].ambient_color[3] = 1.0f;
			(*material_parameters)[i].diffuse_color[3] = 1.0f;
			(*material_parameters)[i].specular_color[3] = 1.0f;
			(*material_parameters)[i].emissive_color[3] = 1.0f;

			(*diffuse_texture_names)[i] = (char*)malloc(sizeof(char) * 256);
			fread((*diffuse_texture_names)[i], sizeof(char), 256, fp);
		}
		*bOnce = true;
	}

	fclose(fp);

	return n_faces;
}

void set_material(Material_Parameters* material_parameters) {
	// assume ShaderProgram_TXPS is used
	glUniform4fv(loc_material.ambient_color, 1, material_parameters->ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_parameters->diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_parameters->specular_color);
	glUniform1f(loc_material.specular_exponent, material_parameters->specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_parameters->emissive_color);
}

void set_material2(Material_Parameters* material_parameters) {
	// assume ShaderProgram_TXPS is used
	glUniform4fv(loc_material_BL.ambient_color, 1, material_parameters->ambient_color);
	glUniform4fv(loc_material_BL.diffuse_color, 1, material_parameters->diffuse_color);
	glUniform4fv(loc_material_BL.specular_color, 1, material_parameters->specular_color);
	glUniform1f(loc_material_BL.specular_exponent, material_parameters->specular_exponent);
	glUniform4fv(loc_material_BL.emissive_color, 1, material_parameters->emissive_color);
}


void bind_texture(GLuint tex, int glTextureId, GLuint texture_name) {
	glActiveTexture(GL_TEXTURE0 + glTextureId);
	glBindTexture(GL_TEXTURE_2D, texture_name);
	glUniform1i(tex, glTextureId);
}


// wolf object
#define N_WOLF_FRAMES 17
GLuint wolf_VBO, wolf_VAO;
int wolf_n_triangles[N_WOLF_FRAMES];
int wolf_vertex_offset[N_WOLF_FRAMES];
GLfloat* wolf_vertices[N_WOLF_FRAMES];

Material_Parameters material_wolf;

void prepare_wolf(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, wolf_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_WOLF_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/wolf/wolf_%02d_vnt.geom", i);
		wolf_n_triangles[i] = read_geometry_vnt(&wolf_vertices[i], n_bytes_per_triangle, filename);

		// assume all geometry files are effective
		wolf_n_total_triangles += wolf_n_triangles[i];

		if (i == 0)
			wolf_vertex_offset[i] = 0;
		else
			wolf_vertex_offset[i] = wolf_vertex_offset[i - 1] + 3 * wolf_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &wolf_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glBufferData(GL_ARRAY_BUFFER, wolf_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_WOLF_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, wolf_vertex_offset[i] * n_bytes_per_vertex,
			wolf_n_triangles[i] * n_bytes_per_triangle, wolf_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_WOLF_FRAMES; i++)
		free(wolf_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &wolf_VAO);
	glBindVertexArray(wolf_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_wolf.ambient_color[0] = 0.5f;
	material_wolf.ambient_color[1] = 0.5f;
	material_wolf.ambient_color[2] = 0.5f;
	material_wolf.ambient_color[3] = 1.0f;

	material_wolf.diffuse_color[0] = 0.500000f;
	material_wolf.diffuse_color[1] = 0.500000f;
	material_wolf.diffuse_color[2] = 0.500000f;
	material_wolf.diffuse_color[3] = 1.0f;

	material_wolf.specular_color[0] = 0.005f;
	material_wolf.specular_color[1] = 0.005f;
	material_wolf.specular_color[2] = 0.005f;
	material_wolf.specular_color[3] = 1.0f;

	material_wolf.specular_exponent = 5.334717f;

	material_wolf.emissive_color[0] = 0.000000f;
	material_wolf.emissive_color[1] = 0.000000f;
	material_wolf.emissive_color[2] = 0.000000f;
	material_wolf.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_WOLF]);

	My_glTexImage2D_from_file("Data/woodentable.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_wolf(void) {
	set_material(&material_wolf);
	bind_texture(loc_diffuse_texture, TEXTURE_INDEX_DIFFUSE, texture_names[TEXTURE_WOLF]);

	glFrontFace(GL_CW);

	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_wolf2(void) {
	set_material2(&material_wolf);
	bind_texture(loc_diffuse_texture, TEXTURE_INDEX_DIFFUSE, texture_names[TEXTURE_WOLF]);

	glFrontFace(GL_CW);

	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}
// dragon object
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;

Material_Parameters material_dragon;

void prepare_dragon(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/dragon_vnt.geom");
	dragon_n_triangles = read_geometry_vnt(&dragon_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	dragon_n_total_triangles += dragon_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(dragon_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_dragon.ambient_color[0] = 0.5f;
	material_dragon.ambient_color[1] = 0.5f;
	material_dragon.ambient_color[2] = 0.5f;
	material_dragon.ambient_color[3] = 1.0f;

	material_dragon.diffuse_color[0] = 0.3f;
	material_dragon.diffuse_color[1] = 0.3f;
	material_dragon.diffuse_color[2] = 0.4f;
	material_dragon.diffuse_color[3] = 1.0f;

	material_dragon.specular_color[0] = 0.3f;
	material_dragon.specular_color[1] = 0.3f;
	material_dragon.specular_color[2] = 0.4f;
	material_dragon.specular_color[3] = 1.0f;

	material_dragon.specular_exponent = 11.334717f;

	material_dragon.emissive_color[0] = 0.000000f;
	material_dragon.emissive_color[1] = 0.000000f;
	material_dragon.emissive_color[2] = 0.000000f;
	material_dragon.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_DRAGON]);

	My_glTexImage2D_from_file("Data/danji.jpg");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
}



void draw_dragon(void) {
	set_material(&material_dragon);
	bind_texture(loc_diffuse_texture, TEXTURE_INDEX_DIFFUSE, texture_names[TEXTURE_DRAGON]);

	glFrontFace(GL_CW);

	glBindVertexArray(dragon_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_dragon2(void) {
	set_material2(&material_dragon);
	bind_texture(loc_diffuse_texture, TEXTURE_INDEX_DIFFUSE, texture_names[TEXTURE_DRAGON]);

	glFrontFace(GL_CW);

	glBindVertexArray(dragon_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_2D, 0);
}


// ironman object
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

void prepare_ironman(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry_vnt(&ironman_vertices, n_bytes_per_triangle, filename);
	ironman_n_triangles = read_geometry_vnt(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;

	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_ironman.ambient_color[0] = 0.4f;
	material_ironman.ambient_color[1] = 0.4f;
	material_ironman.ambient_color[2] = 0.4f;
	material_ironman.ambient_color[3] = 1.0f;

	material_ironman.diffuse_color[0] = 0.900000f;
	material_ironman.diffuse_color[1] = 0.200000f;
	material_ironman.diffuse_color[2] = 0.200000f;
	material_ironman.diffuse_color[3] = 1.0f;

	material_ironman.specular_color[0] = 1.0f;
	material_ironman.specular_color[1] = 1.0f;
	material_ironman.specular_color[2] = 1.0f;
	material_ironman.specular_color[3] = 1.0f;

	material_ironman.specular_exponent = 52.334717f;

	material_ironman.emissive_color[0] = 0.000000f;
	material_ironman.emissive_color[1] = 0.000000f;
	material_ironman.emissive_color[2] = 0.000000f;
	material_ironman.emissive_color[3] = 1.0f;

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_IRONMAN]);

	My_glTexImage2D_from_file("Data/static_objects/Ironman_Body.png");

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void set_material_ironman(void) {
	// assume ShaderProgram_GS is used
	glUseProgram(h_ShaderProgram_GR);
	glUniform4fv(loc_material_GR.ambient_color, 1, material_ironman.ambient_color);
	glUniform4fv(loc_material_GR.diffuse_color, 1, material_ironman.diffuse_color);
	glUniform4fv(loc_material_GR.specular_color, 1, material_ironman.specular_color);
	glUniform1f(loc_material_GR.specular_exponent, material_ironman.specular_exponent);
	glUniform4fv(loc_material_GR.emissive_color, 1, material_ironman.emissive_color);
}
void set_material_ironman_phong(void) {
	// assume ShaderProgram_GS is used
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform4fv(loc_material.ambient_color, 1, material_ironman.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_ironman.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_ironman.specular_color);
	glUniform1f(loc_material.specular_exponent, material_ironman.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_ironman.emissive_color);
}

void draw_ironman(void) {
	//bind_texture(loc_diffuse_texture, TEXTURE_INDEX_DIFFUSE, texture_names[TEXTURE_IRONMAN]);

	glFrontFace(GL_CW);

	glBindVertexArray(ironman_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);

	//glBindTexture(GL_TEXTURE_2D, 0);
}


// cube
GLfloat cube_verts[36][3] = { { -5, -5, 5 },{ 5, -5, 5 },{ 5, 5, 5 },{ 5, 5, 5 },{ -5, 5, 5 },{ -5, -5, 5 },{ 5, -5, -5 },{ -5, -5, -5 },{ -5, 5, -5 },{ -5, 5, -5 },{ 5, 5, -5 },{ 5, -5, -5 },{ -5, -5, -5 },{ -5, -5, 5 },{ -5, 5, 5 },{ -5, 5, 5 },{ -5, 5, -5 },{ -5, -5, -5 },{ 5, -5, 5 },{ 5, -5, -5 },{ 5, 5, -5 },{ 5, 5, -5 },{ 5, 5, 5 },{ 5, -5, 5 },{ -5, 5, 5 },{ 5, 5, 5 },{ 5, 5, -5 },{ 5, 5, -5 },{ -5, 5, -5 },{ -5, 5, 5 },{ -5, -5, -5 },{ 5, -5, -5 },{ 5, -5, 5 },{ 5, -5, 5 },{ -5, -5, 5 },{ -5, -5, -5 } };
GLuint VBO_cube, VAO_cube;
GLfloat cube_color[4] = { 0 / 255.0f, 59 / 255.0f, 111 / 255.0f, cube_alpha };
Material_Parameters material_cube;
void prepare_cube() {
	GLsizeiptr buffer_size = sizeof(cube_verts);

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_cube);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
	glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_STATIC_DRAW); // allocate buffer object memory

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_verts), cube_verts);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_cube);
	glBindVertexArray(VAO_cube);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_cube);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cube.ambient_color[0] = 0.0f;
	material_cube.ambient_color[1] = 0.0f;
	material_cube.ambient_color[2] = 0.0f;
	material_cube.ambient_color[3] = 1.0f;

	material_cube.diffuse_color[0] = 0.000000f;
	material_cube.diffuse_color[1] = 0.000000f;
	material_cube.diffuse_color[2] = 0.000000f;
	material_cube.diffuse_color[3] = 1.0f;

	material_cube.specular_color[0] = 0.0f;
	material_cube.specular_color[1] = 0.0f;
	material_cube.specular_color[2] = 0.0f;
	material_cube.specular_color[3] = 1.0f;

	material_cube.specular_exponent = 1;

	material_cube.emissive_color[0] = 0.000000f;
	material_cube.emissive_color[1] = 0.000000f;
	material_cube.emissive_color[2] = 0.000000f;
	material_cube.emissive_color[3] = 1.0f;

}


void draw_cube(void) {
	set_material(&material_cube);

	glFrontFace(GL_CW);

	glBindVertexArray(VAO_cube);
	glDrawArrays(GL_TRIANGLES, 0, 3 * 36);
	glBindVertexArray(0);
}

void set_material_cube(void) {
	// assume ShaderProgram_BL is used
	glUniform4fv(loc_material_BL.ambient_color, 1, material_cube.ambient_color);
	glUniform4fv(loc_material_BL.diffuse_color, 1, material_cube.diffuse_color);
	glUniform4fv(loc_material_BL.specular_color, 1, material_cube.specular_color);
	glUniform1f(loc_material_BL.specular_exponent, material_cube.specular_exponent);
	glUniform4fv(loc_material_BL.emissive_color, 1, material_cube.emissive_color);
}

void draw_cube2(void) {
	set_material_cube();

	glFrontFace(GL_CW);

	glBindVertexArray(VAO_cube);
	glDrawArrays(GL_TRIANGLES, 0, 3 * 36);
	glBindVertexArray(0);
}

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];

	//simple shader
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	//phong shader
	ShaderInfo shader_info_TXPS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].shadow_on", i);
		loc_light[i].shadow_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");


	loc_material.diffuseTex = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_material.normalTex = glGetUniformLocation(h_ShaderProgram_TXPS, "u_normal_texture");
	loc_material.emissiveTex = glGetUniformLocation(h_ShaderProgram_TXPS, "u_emissive_texture");

	loc_flag_diffuse_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_diffuse_texture_mapping");
	loc_flag_normal_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_normal_texture_mapping");
	loc_flag_emissive_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_emissive_texture_mapping");
	loc_flag_normal_based_directX = glGetUniformLocation(h_ShaderProgram_TXPS, "u_normal_based_directX");
	loc_all_directions = glGetUniformLocation(h_ShaderProgram_TXPS, "u_all_directions"); //erase if something goes wrong


	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");

	//gourad shader

	ShaderInfo shader_info_GR[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_GR = LoadShaders(shader_info_GR);
	loc_ModelViewProjectionMatrix_GR = glGetUniformLocation(h_ShaderProgram_GR, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_GR = glGetUniformLocation(h_ShaderProgram_GR, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_GR = glGetUniformLocation(h_ShaderProgram_GR, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_GR = glGetUniformLocation(h_ShaderProgram_GR, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_GR[i].light_on = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].shadow_on", i);
		loc_light_GR[i].shadow_on = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_GR[i].position = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_GR[i].ambient_color = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_GR[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_GR[i].specular_color = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_GR[i].spot_direction = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_GR[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_GR[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_GR, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_GR[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_GR, string);
	}

	loc_material_GR.ambient_color = glGetUniformLocation(h_ShaderProgram_GR, "u_material.ambient_color");
	loc_material_GR.diffuse_color = glGetUniformLocation(h_ShaderProgram_GR, "u_material.diffuse_color");
	loc_material_GR.specular_color = glGetUniformLocation(h_ShaderProgram_GR, "u_material.specular_color");
	loc_material_GR.emissive_color = glGetUniformLocation(h_ShaderProgram_GR, "u_material.emissive_color");
	loc_material_GR.specular_exponent = glGetUniformLocation(h_ShaderProgram_GR, "u_material.specular_exponent");

	//phong blending shader
	ShaderInfo shader_info_BL[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong_alpha_blending.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/Phong_alpha_blending.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram_BL = LoadShaders(shader_info_BL);
	loc_ModelViewProjectionMatrix_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light_BL[i].light_on = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].shadow_on", i);
		loc_light_BL[i].shadow_on = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light_BL[i].position = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light_BL[i].ambient_color = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light_BL[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light_BL[i].specular_color = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light_BL[i].spot_direction = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light_BL[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light_BL[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_BL, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light_BL[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_BL, string);
	}

	loc_material_BL.ambient_color = glGetUniformLocation(h_ShaderProgram_BL, "u_material.ambient_color");
	loc_material_BL.diffuse_color = glGetUniformLocation(h_ShaderProgram_BL, "u_material.diffuse_color");
	loc_material_BL.specular_color = glGetUniformLocation(h_ShaderProgram_BL, "u_material.specular_color");
	loc_material_BL.emissive_color = glGetUniformLocation(h_ShaderProgram_BL, "u_material.emissive_color");
	loc_material_BL.specular_exponent = glGetUniformLocation(h_ShaderProgram_BL, "u_material.specular_exponent");
	loc_u_flag_blending = glGetUniformLocation(h_ShaderProgram_BL, "u_flag_blending");
	loc_u_fragment_alpha = glGetUniformLocation(h_ShaderProgram_BL, "u_fragment_alpha");

	loc_material_BL.diffuseTex = glGetUniformLocation(h_ShaderProgram_BL, "u_base_texture");
	loc_material_BL.normalTex = glGetUniformLocation(h_ShaderProgram_BL, "u_normal_texture");
	loc_material_BL.emissiveTex = glGetUniformLocation(h_ShaderProgram_BL, "u_emissive_texture");

	loc_flag_diffuse_texture_mapping_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_flag_diffuse_texture_mapping");
	loc_flag_normal_texture_mapping_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_flag_normal_texture_mapping");
	loc_flag_emissive_texture_mapping_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_flag_emissive_texture_mapping");
	loc_flag_normal_based_directX_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_normal_based_directX");

	loc_flag_fog_BL = glGetUniformLocation(h_ShaderProgram_BL, "u_flag_fog");

	//skybox shader
	ShaderInfo shader_info_skybox[3] = {
	{ GL_VERTEX_SHADER, "Shaders/skybox.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/skybox.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram_skybox = LoadShaders(shader_info_skybox);
	loc_cubemap_skybox = glGetUniformLocation(h_ShaderProgram_skybox, "u_skymap");
	loc_ModelViewProjectionMatrix_SKY = glGetUniformLocation(h_ShaderProgram_skybox, "u_ModelViewProjectionMatrix");
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//bistro_exterior
GLuint* bistro_exterior_VBO;
GLuint* bistro_exterior_VAO;
int* bistro_exterior_n_triangles;
int* bistro_exterior_vertex_offset;
GLfloat** bistro_exterior_vertices;
GLuint* bistro_exterior_texture_names;

bool* flag_texture_mapping;

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;
	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	GLenum format, internalFormat;
	if (tx_bits_per_pixel == 32) {
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}
	else if (tx_bits_per_pixel == 24) {
		format = GL_BGR;
		internalFormat = GL_RGB;
	}
	else {
		fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap = FreeImage_ConvertTo32Bits(tx_pixmap);
		format = GL_BGRA;
		internalFormat = GL_RGBA;
	}

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_bistro_exterior(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	bistro_exterior_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	bistro_exterior_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	bistro_exterior_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	bistro_exterior_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	bistro_exterior_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		bistro_exterior_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				bistro_exterior_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		bistro_exterior_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			bistro_exterior_vertex_offset[materialIdx] = 0;
		else
			bistro_exterior_vertex_offset[materialIdx] = bistro_exterior_vertex_offset[materialIdx - 1] + 3 * bistro_exterior_n_triangles[materialIdx - 1];

		glGenBuffers(1, &bistro_exterior_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, bistro_exterior_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			bistro_exterior_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(bistro_exterior_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &bistro_exterior_VAO[materialIdx]);
		glBindVertexArray(bistro_exterior_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, bistro_exterior_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d bistro exterior materials into graphics memory.\n", scene.n_materials);

	// textures
	bistro_exterior_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, bistro_exterior_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded bistro exterior textures into graphics memory.\n");

	free(bistro_exterior_vertices);
}

void bindTexture(GLuint tex, int glTextureId, int texId) {
	if (INVALID_TEX_ID != texId) {
		glActiveTexture(GL_TEXTURE0 + glTextureId);
		glBindTexture(GL_TEXTURE_2D, bistro_exterior_texture_names[texId]);
		glUniform1i(tex, glTextureId);
	}
}

void draw_bistro_exterior(void) {
	glUseProgram(h_ShaderProgram_TXPS);

	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4fv(loc_material.emissive_color, 1, scene.material_list[materialIdx].shading.ph.kr);

		int diffuseTexId = scene.material_list[materialIdx].diffuseTexId;
		int normalTexId = scene.material_list[materialIdx].normalMapTexId;
		int emissiveTexId = scene.material_list[materialIdx].emissiveTexId;

		bindTexture(loc_material.diffuseTex, TEXTURE_INDEX_DIFFUSE, diffuseTexId);
		glUniform1i(loc_flag_diffuse_texture_mapping, flag_texture_mapping[diffuseTexId]);
		bindTexture(loc_material.normalTex, TEXTURE_INDEX_NORMAL, normalTexId);
		glUniform1i(loc_flag_normal_texture_mapping, flag_texture_mapping[normalTexId]);
		glUniform1i(loc_flag_normal_based_directX, 1); // only for bistro exterior
		bindTexture(loc_material.emissiveTex, TEXTURE_INDEX_EMISSIVE, emissiveTexId);
		glUniform1i(loc_flag_emissive_texture_mapping, flag_texture_mapping[emissiveTexId]);

		glEnable(GL_TEXTURE_2D);

		glBindVertexArray(bistro_exterior_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * bistro_exterior_n_triangles[materialIdx]);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glUseProgram(0);
}

// skybox
GLuint skybox_VBO, skybox_VAO;
GLuint skybox_texture_name;

GLfloat cube_vertices[72][3] = {
	// vertices enumerated clockwise
	  // 6*2*3 * 2 (POS & NORM)

	// position
	-1.0f,  1.0f, -1.0f,    1.0f,  1.0f, -1.0f,    1.0f,  1.0f,  1.0f, //right
	 1.0f,  1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f, -1.0f, //left
	 1.0f, -1.0f, -1.0f,   -1.0f, -1.0f,  1.0f,    1.0f, -1.0f,  1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f,  1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //top
	 1.0f,  1.0f,  1.0f,    1.0f, -1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,   -1.0f, -1.0f, -1.0f,    1.0f, -1.0f, -1.0f, //bottom
	 1.0f, -1.0f, -1.0f,    1.0f,  1.0f, -1.0f,   -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,   -1.0f, -1.0f, -1.0f,   -1.0f,  1.0f, -1.0f, //back
	-1.0f,  1.0f, -1.0f,   -1.0f,  1.0f,  1.0f,   -1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,    1.0f, -1.0f,  1.0f,    1.0f,  1.0f,  1.0f, //front
	 1.0f,  1.0f,  1.0f,    1.0f,  1.0f, -1.0f,    1.0f, -1.0f, -1.0f,

	 // normal
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,
	 0.0f, 0.0f, -1.0f,      0.0f, 0.0f, -1.0f,     0.0f, 0.0f, -1.0f,

	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,
	-1.0f, 0.0f,  0.0f,     -1.0f, 0.0f,  0.0f,    -1.0f, 0.0f,  0.0f,

	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,
	 1.0f, 0.0f,  0.0f,      1.0f, 0.0f,  0.0f,     1.0f, 0.0f,  0.0f,

	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,
	 0.0f, 0.0f, 1.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f, 1.0f,

	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,
	 0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 1.0f, 0.0f,

	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f,
	 0.0f, -1.0f, 0.0f,      0.0f, -1.0f, 0.0f,     0.0f, -1.0f, 0.0f
};

void readTexImage2DForCubeMap(const char* filename, GLenum texture_target) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);

	width = FreeImage_GetWidth(tx_pixmap);
	height = FreeImage_GetHeight(tx_pixmap);
	FreeImage_FlipVertical(tx_pixmap);
	data = FreeImage_GetBits(tx_pixmap);

	glTexImage2D(texture_target, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap);
}

void prepare_skybox(void) { // Draw skybox.
	glGenVertexArrays(1, &skybox_VAO);
	glGenBuffers(1, &skybox_VBO);

	glBindVertexArray(skybox_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, skybox_VBO);
	glBufferData(GL_ARRAY_BUFFER, 36 * 3 * sizeof(GLfloat), &cube_vertices[0][0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &skybox_texture_name);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	readTexImage2DForCubeMap("Scene/Cubemap/px.png", GL_TEXTURE_CUBE_MAP_POSITIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/nx.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
	readTexImage2DForCubeMap("Scene/Cubemap/py.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/ny.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
	readTexImage2DForCubeMap("Scene/Cubemap/pz.png", GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
	readTexImage2DForCubeMap("Scene/Cubemap/nz.png", GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
	fprintf(stdout, " * Loaded cube map textures into graphics memory.\n\n");

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void draw_skybox(void) {
	glUseProgram(h_ShaderProgram_skybox);

	glUniform1i(loc_cubemap_skybox, TEXTURE_INDEX_SKYMAP);

	ModelViewMatrix = ViewMatrix * glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	//ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(20000.0f, 20000.0f, 20000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_SKY, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(skybox_VAO);
	glActiveTexture(GL_TEXTURE0 + TEXTURE_INDEX_SKYMAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_name);

	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 3);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);
}
/*****************************  END: geometry setup *****************************/

/********************************  START: light *********************************/
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];

void initialize_lights(void) { // follow OpenGL conventions for initialization

	//light 0
	light[0].light_on = 1;

	light[0].position[0] = scene.light_list[0].pos[0];
	light[0].position[1] = scene.light_list[0].pos[1];
	light[0].position[2] = scene.light_list[0].pos[2];
	light[0].position[3] = 0.0f;

	light[0].ambient_color[0] = 0.0f;
	light[0].ambient_color[1] = 0.0f;
	light[0].ambient_color[2] = 0.0f;
	light[0].ambient_color[3] = 1.0f;

	light[0].diffuse_color[0] = scene.light_list[0].color[0];
	light[0].diffuse_color[1] = scene.light_list[0].color[1];
	light[0].diffuse_color[2] = scene.light_list[0].color[2];
	light[0].diffuse_color[3] = 1.0f;

	light[0].specular_color[0] = 0.5f; //0.5
	light[0].specular_color[1] = 0.5f; //0.5
	light[0].specular_color[2] = 0.5f; //0.5
	light[0].specular_color[3] = 1.0f;


	//light 1 - saesang
	light[1].light_on = 1;
	light[1].position[0] = -150.0;
	light[1].position[1] = 1100.0;
	light[1].position[2] = 1000;
	light[1].position[3] = 1;

	light[1].ambient_color[0] = 1.0f;
	light[1].ambient_color[1] = 0.0f;
	light[1].ambient_color[2] = 1.0f;
	light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 1.0f;
	light[1].diffuse_color[1] = 0.0f;
	light[1].diffuse_color[2] = 1.0f;
	light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 1.0f;
	light[1].specular_color[1] = 0.0f;
	light[1].specular_color[2] = 1.0f;
	light[1].specular_color[3] = 1.0f;

	light[1].light_attenuation_factors[0] = 1.0f;
	light[1].light_attenuation_factors[1] = 0.0f;
	light[1].light_attenuation_factors[2] = 0.0f;
	light[1].light_attenuation_factors[3] = 0.0f;

	light[1].spot_direction[0] = 0.0f;
	light[1].spot_direction[1] = 0.0f;
	light[1].spot_direction[2] = -1.0f;

	light[1].spot_cutoff_angle = 30.0f;
	light[1].spot_exponent = 1.0f;


	//light 2 - wolf eyesight
	light[2].light_on = 1;
	light[2].position[0] = 0.0;
	light[2].position[1] = 0.0; //20, 100
	light[2].position[2] = 30.0; //30
	light[2].position[3] = 1;

	light[2].ambient_color[0] = 0.0f;
	light[2].ambient_color[1] = 0.0f;
	light[2].ambient_color[2] = 1.0f;
	light[2].ambient_color[3] = 1.0f;

	light[2].diffuse_color[0] = 0.0f;
	light[2].diffuse_color[1] = 0.0f;
	light[2].diffuse_color[2] = 1.0f;
	light[2].diffuse_color[3] = 1.0f;

	light[2].specular_color[0] = 0.0f;
	light[2].specular_color[1] = 0.0f;
	light[2].specular_color[2] = 1.0f;
	light[2].specular_color[3] = 1.0f;

	light[2].light_attenuation_factors[0] = 1.0f;
	light[2].light_attenuation_factors[1] = 0.0f;
	light[2].light_attenuation_factors[2] = 0.0f;
	light[2].light_attenuation_factors[3] = 0.0f;

	light[2].spot_direction[0] = 0.0f;
	light[2].spot_direction[1] = 0.0f;
	light[2].spot_direction[2] = 1.0f;

	light[2].spot_cutoff_angle = 45.0f;
	light[2].spot_exponent = 8.0f;


	//light 3 - me
	light[3].light_on = 1;
	light[3].position[0] = 0.0;
	light[3].position[1] = 0.0; //20, 100
	light[3].position[2] = 30.0; //30
	light[3].position[3] = 1;

	light[3].ambient_color[0] = 0.8f;
	light[3].ambient_color[1] = 0.8;
	light[3].ambient_color[2] = 0.8;
	light[3].ambient_color[3] = 1.0f;

	light[3].diffuse_color[0] = 0.8;
	light[3].diffuse_color[1] = 0.8;
	light[3].diffuse_color[2] = 0.8;
	light[3].diffuse_color[3] = 1.0f;

	light[3].specular_color[0] = 0.8;
	light[3].specular_color[1] = 0.8;
	light[3].specular_color[2] = 0.8;
	light[3].specular_color[3] = 1.0f;

	light[3].light_attenuation_factors[0] = 1.0f;
	light[3].light_attenuation_factors[1] = 0.0f;
	light[3].light_attenuation_factors[2] = 0.0f;
	light[3].light_attenuation_factors[3] = 0.0f;

	light[3].spot_direction[0] = 0.0f;
	light[3].spot_direction[1] = 0.0f;
	light[3].spot_direction[2] = -1.0f;

	light[3].spot_cutoff_angle = 30.0f;
	light[3].spot_exponent = 2.0f;

}

void set_lights(void) {
	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 0.7f, 0.7f, 0.7f, 1.0f);
	//glUniform4f(loc_global_ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);


	glm::vec4 light_position_EC;
	glm::vec3 light_direction_EC;
	//light0
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform1i(loc_light[0].shadow_on, light[0].shadow_on);

	light_position_EC = ViewMatrix * glm::vec4(light[0].position[0], light[0].position[1], light[0].position[2], light[0].position[3]);
	glUniform4fv(loc_light[0].position, 1, &light_position_EC[0]);

	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);


	//light1 - saesang
	glUniform1i(loc_light[1].light_on, light[1].light_on);
	glUniform1i(loc_light[1].shadow_on, light[1].shadow_on);
	glUniform1i(loc_all_directions, all_directions); //delete if something goes wrong

	light_position_EC = ViewMatrix * glm::vec4(light[1].position[0], light[1].position[1], light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light[1].position, 1, &light_position_EC[0]);

	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	light_direction_EC = glm::mat3(ViewMatrix) * (glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1], light[1].spot_direction[2]));
	glUniform3fv(loc_light[1].spot_direction, 1, &light_direction_EC[0]);
	glUniform1f(loc_light[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light[1].spot_exponent, light[1].spot_exponent);
	glUniform4fv(loc_light[1].light_attenuation_factors, 1, light[1].light_attenuation_factors);


	//light2
	glUniform1i(loc_light[2].light_on, light[2].light_on);
	glUniform1i(loc_light[2].shadow_on, light[2].shadow_on);

	light_position_EC = WolfModelViewMatrix * glm::vec4(light[2].position[0], light[2].position[1], light[2].position[2], light[2].position[3]);
	glUniform4fv(loc_light[2].position, 1, &light_position_EC[0]);

	glUniform4fv(loc_light[2].ambient_color, 1, light[2].ambient_color);
	glUniform4fv(loc_light[2].diffuse_color, 1, light[2].diffuse_color);
	glUniform4fv(loc_light[2].specular_color, 1, light[2].specular_color);
	light_direction_EC = glm::mat3(WolfModelViewMatrixInvTrans) * (glm::vec3(light[2].spot_direction[0], light[2].spot_direction[1], light[2].spot_direction[2]));
	glUniform3fv(loc_light[2].spot_direction, 1, &light_direction_EC[0]);
	glUniform1f(loc_light[2].spot_cutoff_angle, light[2].spot_cutoff_angle);
	glUniform1f(loc_light[2].spot_exponent, light[2].spot_exponent);
	glUniform4fv(loc_light[2].light_attenuation_factors, 1, light[2].light_attenuation_factors);

	//light3
	glUniform1i(loc_light[3].light_on, light[3].light_on);
	glUniform1i(loc_light[3].shadow_on, light[3].shadow_on);
	glUniform4fv(loc_light[3].position, 1, light[3].position);
	glUniform4fv(loc_light[3].ambient_color, 1, light[3].ambient_color);
	glUniform4fv(loc_light[3].diffuse_color, 1, light[3].diffuse_color);
	glUniform4fv(loc_light[3].specular_color, 1, light[3].specular_color);
	glUniform3fv(loc_light[3].spot_direction, 1, light[3].spot_direction);
	glUniform1f(loc_light[3].spot_cutoff_angle, light[3].spot_cutoff_angle);
	glUniform1f(loc_light[3].spot_exponent, light[3].spot_exponent);
	glUniform4fv(loc_light[3].light_attenuation_factors, 1, light[3].light_attenuation_factors);

	glUseProgram(h_ShaderProgram_GR);
	glUniform4f(loc_global_ambient_color_GR, 0.7f, 0.7f, 0.7f, 1.0f);
	//light0 Gouraud
	glUniform1i(loc_light_GR[0].light_on, light[0].light_on);
	glUniform1i(loc_light_GR[0].shadow_on, light[0].shadow_on);

	light_position_EC = ViewMatrix * glm::vec4(light[0].position[0], light[0].position[1], light[0].position[2], light[0].position[3]);
	glUniform4fv(loc_light_GR[0].position, 1, &light_position_EC[0]);

	glUniform4fv(loc_light_GR[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light_GR[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light_GR[0].specular_color, 1, light[0].specular_color);

	glUseProgram(0);

	glUseProgram(h_ShaderProgram_BL);
	glUniform4f(loc_global_ambient_color_BL, 0.7f, 0.7f, 0.7f, 1.0f);
	//light0 Gouraud
	glUniform1i(loc_light_BL[0].light_on, light[0].light_on);
	glUniform1i(loc_light_BL[0].shadow_on, light[0].shadow_on);

	light_position_EC = ViewMatrix * glm::vec4(light[0].position[0], light[0].position[1], light[0].position[2], light[0].position[3]);
	glUniform4fv(loc_light_BL[0].position, 1, &light_position_EC[0]);

	glUniform4fv(loc_light_BL[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light_BL[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light_BL[0].specular_color, 1, light[0].specular_color);

	glUseProgram(0);
}
/*********************************  END: light **********************************/

void display_blend(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(h_ShaderProgram_TXPS);

	set_lights();

	draw_grid();
	draw_axes();
	draw_bistro_exterior();
	draw_skybox();


	//glUseProgram(h_ShaderProgram_TXPS);
	//glUseProgram(h_ShaderProgram_GR);
	if (gouraud == 1) set_material_ironman();
	else if (phong == 1) {
		set_material_ironman_phong();
	}
	//draw ironman
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-290, -100, 110));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(ironmanScale, ironmanScale, ironmanScale));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 190 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniform1i(loc_flag_normal_texture_mapping, false);
	glUniform1i(loc_flag_emissive_texture_mapping, false);
	glUniform1i(loc_flag_diffuse_texture_mapping, false);
	//glUniform3f(loc_primitive_color, 170 / 255.0, 5 / 255.0, 5 / 255.0);
	if (gouraud == 1) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GR, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GR, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GR, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	}
	else if (phong == 1) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	}
	draw_ironman();

	glUseProgram(h_ShaderProgram_TXPS);

	//draw_wolf
	int wolf_clock = _timestamp_scene % 1440;
	if (wolf_clock <= 360) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, wolf_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
	}
	else if (wolf_clock <= 720) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -(wolf_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
	}
	else if (wolf_clock <= 1080) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0, -WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -(wolf_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0, WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (wolf_clock <= 1440) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0, WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (wolf_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0, -WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	WolfModelViewMatrix = ModelViewMatrix;
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(900.0f, 900.0f, 900.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	//WolfModelViewMatrix = glm::rotate(WolfModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	WolfModelViewMatrixInvTrans = ModelViewMatrixInvTrans;
	glUniform1i(loc_flag_normal_texture_mapping, false);
	glUniform1i(loc_flag_emissive_texture_mapping, false);
	glUniform1i(loc_flag_diffuse_texture_mapping, true);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wolf();


	//draw dragon;
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500, -3000, 0));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(dragonScale, dragonScale, dragonScale));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniform1i(loc_flag_normal_texture_mapping, false);
	glUniform1i(loc_flag_emissive_texture_mapping, false);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_dragon();



	//glDepthFunc(GL_GREATER);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(h_ShaderProgram_BL);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glUniform1i(loc_u_flag_blending, 1);
	glUniform1f(loc_u_fragment_alpha, cube_alpha);
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, 16000)); //-500,-500,500
	ModelViewMatrix = glm::rotate(ModelViewMatrix, cube_degree * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(35, 35, 35));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniform1i(loc_flag_diffuse_texture_mapping, false);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	draw_cube2();
	glCullFace(GL_BACK);
	draw_cube2();
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);


	//glUseProgram(h_ShaderProgram_TXPS);



	glutSwapBuffers();
}

/********************  START: callback function definitions *********************/
void display(void) {
	if (blendFlag == 1) {
		display_blend();
		return;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	set_lights();

	draw_grid();
	draw_axes();
	draw_bistro_exterior();
	draw_skybox();

	//glUseProgram(h_ShaderProgram_TXPS);
	//glUseProgram(h_ShaderProgram_GR);
	if (gouraud == 1) set_material_ironman();
	else if (phong == 1) {
		set_material_ironman_phong();
	}
	//draw ironman
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-290, -100, 110));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(ironmanScale, ironmanScale, ironmanScale));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 190 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniform1i(loc_flag_normal_texture_mapping, false);
	glUniform1i(loc_flag_emissive_texture_mapping, false);
	glUniform1i(loc_flag_diffuse_texture_mapping, false);
	//glUniform3f(loc_primitive_color, 170 / 255.0, 5 / 255.0, 5 / 255.0);
	if (gouraud == 1) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_GR, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_GR, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_GR, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	}
	else if (phong == 1) {
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
		glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	}
	draw_ironman();

	glUseProgram(h_ShaderProgram_TXPS);
	//glUseProgram(h_ShaderProgram_simple);

	//draw_wolf
	int wolf_clock = _timestamp_scene % 1440;
	if (wolf_clock <= 360) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, wolf_clock * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(-WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
	}
	else if (wolf_clock <= 720) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -(wolf_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(WOLF_ROTATION_RADIUS, 0.0f, 0.0f));
	}
	else if (wolf_clock <= 1080) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0, -WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -(wolf_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0, WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (wolf_clock <= 1440) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0, WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (wolf_clock)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::translate(ModelViewMatrix, glm::vec3(0.0, -WOLF_ROTATION_RADIUS, 0.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	WolfModelViewMatrix = ModelViewMatrix;
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(900.0f, 900.0f, 900.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	//WolfModelViewMatrix = glm::rotate(WolfModelViewMatrix, 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	WolfModelViewMatrixInvTrans = ModelViewMatrixInvTrans;

	glUniform1i(loc_flag_normal_texture_mapping, false);
	glUniform1i(loc_flag_emissive_texture_mapping, false);
	glUniform1i(loc_flag_diffuse_texture_mapping, true);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_wolf();


	//draw dragon;
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500, -3000, 0));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(dragonScale, dragonScale, dragonScale));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniform1i(loc_flag_normal_texture_mapping, false);
	glUniform1i(loc_flag_emissive_texture_mapping, false);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_dragon();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0, 0, 16000)); //-500,-500,500
	ModelViewMatrix = glm::rotate(ModelViewMatrix, cube_degree * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(35, 35, 35));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::inverseTranspose(glm::mat3(ModelViewMatrix));
	glUniform1i(loc_flag_diffuse_texture_mapping, false);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	draw_cube();


	glutSwapBuffers();
}






void rotateCamV_20181200(int angle) {
	glm::mat3 RotMat;
	glm::vec3 dir;

	RotMat = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle, glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2])));

	dir = RotMat * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
	current_camera.uaxis[0] = dir.x; current_camera.uaxis[1] = dir.y; current_camera.uaxis[2] = dir.z;
	dir = RotMat * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
	current_camera.naxis[0] = dir.x; current_camera.naxis[1] = dir.y; current_camera.naxis[2] = dir.z;
}

void rotateCamU_20181200(int angle) {
	glm::mat3 RotMat;
	glm::vec3 dir;

	RotMat = glm::mat3(glm::rotate(glm::mat4(1.0), CAM_RSPEED * TO_RADIAN * angle, glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2])));

	dir = RotMat * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
	current_camera.vaxis[0] = dir.x; current_camera.vaxis[1] = dir.y; current_camera.vaxis[2] = dir.z;
	dir = RotMat * glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2]);
	current_camera.naxis[0] = dir.x; current_camera.naxis[1] = dir.y; current_camera.naxis[2] = dir.z;
}

void moveCam_20181200(int key) {
	switch (key) {
	case 'S':
	case 's':
		current_camera.pos[0] -= current_camera.naxis[0] * MOVE_SPEED; //go forward
		current_camera.pos[1] -= current_camera.naxis[1] * MOVE_SPEED; //go forward
		current_camera.pos[2] -= current_camera.naxis[2] * MOVE_SPEED; //go forward
		break;
	case 'X':
	case 'x':
		current_camera.pos[0] += current_camera.naxis[0] * MOVE_SPEED; //go back
		current_camera.pos[1] += current_camera.naxis[1] * MOVE_SPEED; //go forward
		current_camera.pos[2] += current_camera.naxis[2] * MOVE_SPEED; //go forward
		break;
	case 'Z':
	case 'z':
		current_camera.pos[0] -= current_camera.uaxis[0] * MOVE_SPEED; //move left
		current_camera.pos[1] -= current_camera.uaxis[1] * MOVE_SPEED; //go forward
		current_camera.pos[2] -= current_camera.uaxis[2] * MOVE_SPEED; //go forward
		break;
	case 'C':
	case 'c':
		current_camera.pos[0] += current_camera.uaxis[0] * MOVE_SPEED; //move right
		current_camera.pos[1] += current_camera.uaxis[1] * MOVE_SPEED; //go forward
		current_camera.pos[2] += current_camera.uaxis[2] * MOVE_SPEED; //go forward
		break;
	case ' ':
		current_camera.pos[0] += current_camera.vaxis[0] * MOVE_SPEED; //go up
		current_camera.pos[1] += current_camera.vaxis[1] * MOVE_SPEED; //go forward
		current_camera.pos[2] += current_camera.vaxis[2] * MOVE_SPEED; //go forward
		break;
	case 'V':
	case 'v':
		current_camera.pos[0] -= current_camera.vaxis[0] * MOVE_SPEED; //go down
		current_camera.pos[1] -= current_camera.vaxis[1] * MOVE_SPEED; //go forward
		current_camera.pos[2] -= current_camera.vaxis[2] * MOVE_SPEED; //go forward
		break;
	}
}

void rotateCamN_20181200(int key) { // CHECK
	glm::mat3 RotMat;
	glm::vec3 dir;

	switch (key) {
	case 'Q':
	case 'q':
		RotMat = glm::mat3(glm::rotate(glm::mat4(1.0), TO_RADIAN * 3, glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])));
		break;
	case 'E':
	case 'e':
		RotMat = glm::mat3(glm::rotate(glm::mat4(1.0), TO_RADIAN * -3, glm::vec3(current_camera.naxis[0], current_camera.naxis[1], current_camera.naxis[2])));
		break;
	}

	dir = RotMat * glm::vec3(current_camera.vaxis[0], current_camera.vaxis[1], current_camera.vaxis[2]);
	current_camera.vaxis[0] = dir.x; current_camera.vaxis[1] = dir.y; current_camera.vaxis[2] = dir.z;
	dir = RotMat * glm::vec3(current_camera.uaxis[0], current_camera.uaxis[1], current_camera.uaxis[2]);
	current_camera.uaxis[0] = dir.x; current_camera.uaxis[1] = dir.y; current_camera.uaxis[2] = dir.z;
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	case '1':
		//set_current_camera(CAMERA_1);
		//glutPostRedisplay();
		gouraud = 1;
		phong = 0;
		break;
	case '2':
		//set_current_camera(CAMERA_2);
		//glutPostRedisplay();
		phong = 1;
		gouraud = 0;
		break;
	case '3':
		//set_current_camera(CAMERA_3);
		//glutPostRedisplay();
		light[1].light_on = 1 - light[1].light_on;
		break;
	case '4':
		light[3].light_on = 1 - light[3].light_on;
		break;
	case '5':
		//set_current_camera(CAMERA_5);
		//glutPostRedisplay();
		light[2].light_on = 1 - light[2].light_on;
		break;
	case '6':
		blendFlag = 1 - blendFlag;
		break;
	case 'U':
	case 'u':
		set_current_camera(CAMERA_u);
		glutPostRedisplay();
		break;
	case 'I':
	case 'i':
		set_current_camera(CAMERA_i);
		glutPostRedisplay();
		break;
	case 'O':
	case 'o':
		set_current_camera(CAMERA_o);
		glutPostRedisplay();
		break;
	case 'P':
	case 'p':
		set_current_camera(CAMERA_p);
		glutPostRedisplay();
		break;
	case 'A':
	case 'a':
		set_current_camera(CAMERA_a);
		glutPostRedisplay();
		break;
	case 'B':
	case 'b':
		set_current_camera(CAMERA_6);
		glutPostRedisplay();
		break;
	case 'S':
	case 's':
	case 'X':
	case 'x':
	case 'Z':
	case 'z':
	case 'C':
	case 'c':
	case 'V':
	case 'v':
	case ' ':
		moveCam_20181200(key);
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case 'Q':
	case 'q':
	case 'E':
	case 'e':
		rotateCamN_20181200(key);
		set_ViewMatrix_from_camera_frame();
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
		break;
	case 'R':
	case 'r':
		set_current_camera(CAMERA_4);
		glutPostRedisplay();
		break;
	case 'T':
	case 't':
		set_current_camera(CAMERA_1);
		glutPostRedisplay();
		break;
	case 'Y':
	case 'y':
		all_directions = 1 - all_directions;
		break;
	case 'K':
	case 'k':
		if (blendFlag == 1 && cube_alpha >= 0.1) cube_alpha -= 0.05;
		break;
	case 'L':
	case 'l':
		if (blendFlag == 1 && cube_alpha <= 1.0) cube_alpha += 0.05;
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_CTRL_L:
		ctrl_pressed = 1;
		break;
	case GLUT_KEY_SHIFT_L:
		shift_pressed = 1;
		break;
	}
}

void specialup(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_CTRL_L:
		ctrl_pressed = 0;
		break;
	case GLUT_KEY_SHIFT_L:
		shift_pressed = 0;
		break;
	}
}

float prevx, prevy;
void mousepress(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		leftbuttonpressed = 1;
		prevx = x; prevy = y;
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftbuttonpressed = 0;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		rightbuttonpressed = 1;
		prevx = x; prevy = y;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {
		rightbuttonpressed = 0;
	}

	if (button == 3) {
		if (ctrl_pressed == 1) {
			current_camera.fovy = current_camera.fovy * 0.9;
			ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
	}
	else if (button == 4) {
		if (ctrl_pressed == 1) {
			current_camera.fovy *= 1.1;
			if (current_camera.fovy >= 2) {
				current_camera.fovy = 2;
			}
			ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
	}
}

void mousemove(int x, int y) {
	if (leftbuttonpressed && !shift_pressed) {
		rotateCamV_20181200(prevx - x);
	}
	//else if (leftbuttonpressed & shift_pressed) {
	//	rotateCamN_20181200(prevx - x);
	//}
	if (rightbuttonpressed) {
		rotateCamU_20181200(prevy - y);
	}

	prevx = x; prevy = y;

	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	glutPostRedisplay();
}

void timer_scene(int value) {
	cur_frame_wolf = _timestamp_scene % N_WOLF_FRAMES;
	cube_degree += cube_dx;
	glutPostRedisplay();
	_timestamp_scene = (_timestamp_scene + 5) % UINT_MAX;


	glutTimerFunc(100, timer_scene, 0); //100 = 1 second?
}

void reshape(int width, int height) {
	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, bistro_exterior_VAO);
	glDeleteBuffers(scene.n_materials, bistro_exterior_VBO);

	glDeleteVertexArrays(1, &skybox_VAO);
	glDeleteBuffers(1, &skybox_VBO);

	glDeleteTextures(scene.n_textures, bistro_exterior_texture_names);

	free(bistro_exterior_n_triangles);
	free(bistro_exterior_vertex_offset);

	free(bistro_exterior_VAO);
	free(bistro_exterior_VBO);

	free(bistro_exterior_texture_names);
	free(flag_texture_mapping);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialup);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutTimerFunc(100, timer_scene, 0);
}

void initialize_flags(void) {
	flag_fog = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
	glUseProgram(0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
	initialize_flags();

	glGenTextures(N_TEXTURES_USED, texture_names);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_bistro_exterior();
	prepare_wolf();
	prepare_cube();
	prepare_dragon();
	prepare_ironman();
	prepare_skybox();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
	initialize_camera();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 9
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Bistro Exterior Scene";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'1' : set the camera for original view",
		"		'2' : set the camera for bistro view",
		"		'3' : set the camera for tree view",
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
