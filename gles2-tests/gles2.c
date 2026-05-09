#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "shader_utils.h"
#include "gles2.h"

/* */

static GLuint ShaderProgs;
static GLint positionLoc;
static GLint texCoordLoc;
static GLint rtmatrixLoc;
static GLuint textureId;

static GLfloat angle = 0.0;
static GLfloat delta = 0.01;

#define TW 128
#define TH 128

static GLuint bitmap[TW][TH];

/* */

static GLuint load_shader(const char *shaderSrc, GLenum type)
{
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);

    if (shader == 0)
        return 0;

    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 0) {
            char *infoLog = malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            fprintf(stderr,"Error compiling shader:\n%s\n", infoLog);
            free(infoLog);

            glDeleteShader(shader);
            return 0;
        }
    }

    return shader;
}

static int init_shaders(void)
{
    char *vShaderStr;
    char *fShaderStr;

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

	/* load shaders code from files */
	fShaderStr = shader_fread(FRAGMENT_SHADER);

	if (!fShaderStr) {
		printf("failed to load %s shader\n", FRAGMENT_SHADER);
		return 0;
	}

	vShaderStr = shader_fread(VERTEX_SHADER);

	if (!vShaderStr) {
		printf("failed to load %s shader\n", VERTEX_SHADER);
		return 0;
	}

    /* load vertex/frag shader */
    vertexShader = load_shader(vShaderStr, GL_VERTEX_SHADER);
    fragmentShader = load_shader(fShaderStr, GL_FRAGMENT_SHADER);

    /* create the program object */
    programObject = glCreateProgram();

    if (programObject == 0)
        return 0;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    /* bind vPosition to attrib 0 */
    glBindAttribLocation(programObject, 0, "vPosition");
	glBindAttribLocation(programObject, 1, "myUV");

    /* linkage */
    glLinkProgram(programObject);

    /* check status */
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetShaderiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0) {
            char *infoLog = malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(programObject, infoLen, NULL, infoLog);
            fprintf(stderr,"Error linking shader:\n%s\n", infoLog);
            free(infoLog);

            return 0;
        }
    }

    ShaderProgs = programObject;

	rtmatrixLoc = glGetUniformLocation(ShaderProgs, "myPMVMatrix");
	positionLoc = glGetAttribLocation(ShaderProgs, "vPosition");
	texCoordLoc = glGetAttribLocation(ShaderProgs, "myUV");

    return GL_TRUE;
}

/* */

void gles2_update_state(int val)
{
	/* update texture */

	GLuint * buffer = (GLuint *) bitmap;
	int i,j;

	for (i = 0; i < TH; i++) {
		for (j = 0; j < TW; j++) {
			GLuint col = (val % 250) << 8;
			*(buffer + j + i*TW) = col;
		}
	}

	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, TW, TH, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

	/* update angle */

	angle += delta;
}

void gles2_draw(int width, int height)
{
	float pfIdentity[] = {
		cos(angle), -sin(angle), 0, 0,
		sin(angle), cos(angle), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	GLfloat rectangle_vertices[12] = {
		+0.5, +0.5,	+0.0,	// lower left
		-0.5, +0.5, +0.0,	// lower right
		-0.5, -0.5, +0.0,	// upper right
		+0.5, -0.5, +0.0,	// upper left
	};

	GLfloat texture_coordinates[8] = {
		1.0, 0.0,
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
	};

	GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ShaderProgs);

	glUniformMatrix4fv(rtmatrixLoc, 1, GL_FALSE, pfIdentity);

	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, &rectangle_vertices);
	glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, &texture_coordinates);

	glEnableVertexAttribArray(positionLoc);
	glEnableVertexAttribArray(texCoordLoc);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, textureId);

	glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void gles2_init(void)
{
	init_shaders();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
