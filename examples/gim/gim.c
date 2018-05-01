/*
 *  gim: Program for demonstrating the usage of the OpenGI library
 *  Copyright (C) 2009-2011  Christian Rau
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Christian Rau
 *
 *     rauy@users.sourceforge.net
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <GI/gi.h>
#include <GL/glut.h>
#include <GL/glext.h>

#ifndef M_PI
	#define M_PI			3.1415926535897932
#endif
#define M_TWO_PI			6.2831853071795865

#define VEC3_ADD(d,v,w)		(d)[0]=(v)[0]+(w)[0]; (d)[1]=(v)[1]+(w)[1]; (d)[2]=(v)[2]+(w)[2]
#define VEC3_SUB(d,v,w)		(d)[0]=(v)[0]-(w)[0]; (d)[1]=(v)[1]-(w)[1]; (d)[2]=(v)[2]-(w)[2]
#define VEC3_CROSS(d,v,w)	(d)[0]=(v)[1]*(w)[2] - (v)[2]*(w)[1]; \
							(d)[1]=(v)[2]*(w)[0] - (v)[0]*(w)[2]; \
							(d)[2]=(v)[0]*(w)[1] - (v)[1]*(w)[0]
#define VEC3_NORMALIZE(v)	{ float n=sqrt((v)[0]*(v)[0]+(v)[1]*(v)[1]+(v)[2]*(v)[2]); \
							if(fabs(n)>1e-6) { float m=1.0f/n; (v)[0]*=m; (v)[1]*=m; (v)[2]*=m; } }

unsigned int uiMesh, uiGIM[3], uiList, uiPattern;
float fAngleX = 0.0f, fAngleY = 0.0f, fZ = 5.0f;
int iLastX, iLastY, iButton, iWidth, iHeight;
int iWireframe = 0, iDrawMesh = 1, iParam = 0, iFPS = 0;

void initGL();
unsigned int create_mesh(const char *filename);
unsigned int create_mesh_obj (const char *filename);
int cut_mesh(unsigned int mesh);
int parameterize_mesh(unsigned int mesh, int res);
int create_gim(unsigned int *gim, int res);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void GICALLBACK errorCB(unsigned int error, void *data);

/* simple geometry image creation and reconstruction */
int main(int argc, char *argv[])
{
	GIcontext pContext;
	int i, f = 0, iRes = 0, iTex[3];
	float fRadius;

	for(i=1; i<argc; ++i)
	{
		if(!strcmp(argv[i], "--help"))
		{
			printf("usage: gim [--help] -r <RES> <FILE>\n"
				"  <RES>  resolution of geometry image\n  <FILE> PLY2 file\n"
				"keys for controlling the application:\n"
				"  C   change culling mode\n  G   toggle use of geometry shader\n"
				"  M   switch between geometry image and input mesh\n"
				"  P   show parameterization\n  T   toggle use of vertex texturing\n"
				"  V   view all\n  W   show wireframe overlay\n  ESC exit\n");
			return 0;
		}
		if(!strcmp(argv[i], "-r"))
			iRes = atoi(argv[++i]);
		else
			f = i;
	}
	if(argc < 4)
	{
		printf("usage: gim [--help] -r <RES> <FILE>\n"
			"  <RES>  resolution of geometry image\n  <FILE> PLY2 file\n");
		return -1;
	}
	if(!f)
	{
		fprintf(stderr, "no file specified\n");
		return -1;
	}
	if(iRes < 2 || iRes > 1024)
	{
		fprintf(stderr, "resolution not in [2,1024]\n");
		return -1;
	}

	/* init GLUT and GL */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Geometry Images");
	initGL();

	/* init GI, read mesh and create geometry image */
	pContext = giCreateContext();
	giMakeCurrent(pContext);
	giErrorCallback(errorCB, NULL);
	printf("creating mesh...\n");
	if(!(uiMesh = create_mesh_obj(argv[f])))
	{
		fprintf(stderr, "mesh creation failed!\n");
		return -1;
	}
	printf("cutting...\n");
	if(cut_mesh(uiMesh) == -1)
	{
		fprintf(stderr, "cutting failed!\n");
		return -1;
	}
	printf("parameterizing...\n");
	if(parameterize_mesh(uiMesh, 17) == -1)
	{
		fprintf(stderr, "parameterization failed!\n");
		return -1;
	}
	printf("sampling...\n");
	if(create_gim(uiGIM, iRes) == -1)
	{
		fprintf(stderr, "geometry image creation failed!\n");
		return -1;
	}
	giGLAttribRenderParameteri(2, GI_TEXTURE_COORD_DOMAIN, GI_UNIT_SQUARE);

	/* draw mesh into display list */
	giGLAttribRenderParameteri(0, GI_GL_RENDER_SEMANTIC, GI_GL_VERTEX);
	giGLAttribRenderParameteri(1, GI_GL_RENDER_SEMANTIC, GI_GL_NORMAL);
	giGLAttribRenderParameteri(2, GI_GL_RENDER_SEMANTIC, GI_GL_TEXTURE_COORD);
	giGLAttribRenderParameteri(2, GI_GL_RENDER_CHANNEL, 0);
	glNewList(uiList, GL_COMPILE);
		giGLDrawMesh();
	glEndList();
	giGetMeshfv(GI_RADIUS, &fRadius);
	fZ = fRadius * 1.9f;

	/* run GLUT */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(display);
	glutMainLoop();

	/* clean up */
	giDeleteMesh(uiMesh);
	for(i=0; i<3; ++i)
	{
		giBindImage(uiGIM[i]);
		giGetImageiv(GI_GL_IMAGE_TEXTURE, iTex+i);
	}
	giDeleteImages(3, uiGIM);
	giDestroyContext(pContext);
	glDeleteTextures(3, iTex);
	return 0;
}

/* initialize OpenGL */
void initGL()
{
	static const float specular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	unsigned char checkerboard[256][256];
	int i, j;

	/* set OpenGL state */
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glPolygonOffset(-1.0f, -1.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 96.0f);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	uiList = glGenLists(1);

	/* make texture for param visiualization */
	for (i=0; i<256; ++i)
		for(j=0; j<256; ++j)
			checkerboard[i][j] = 255 * (((i>>3)+(j>>3))&1);
	glGenTextures(1, &uiPattern);
	glBindTexture(GL_TEXTURE_2D, uiPattern);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, 
		GL_LUMINANCE, GL_UNSIGNED_BYTE, checkerboard);
}

unsigned int create_mesh_obj (const char *filename)
{
  unsigned int uiMesh;
  FILE *pFile = fopen (filename, "r");
  float *pVertices, *pNormals;
  unsigned int *pIndices;
  int iNumVertices, iNumIndices;

  if (!pFile)
  {
    fprintf (stderr, "cannot open file!\n");
    return 0;
  }

  char line[256];
  float x, y, z;
  float r, g, b;
  int i1, i2, i3, in1, in2, in3;

  iNumVertices = 0;
  iNumIndices = 0;

  while (fgets (line, 256, pFile))
  {
    if (strlen (line) == 0)
    {
      continue;
    }

    if (line[0] == 'v')
    {
      // normal
      if (line[1] == 'n')
      {
      }
      else
      {
        iNumVertices++;
      }
    }
    else if (line[0] == 'f')
    {
      iNumIndices++;
    }
  }

  iNumVertices *= 3;
  iNumIndices *= 3;
  pVertices = (float*)malloc (iNumVertices * sizeof (float));
  pNormals = (float*)calloc (iNumVertices, sizeof (float));
  pIndices = (unsigned int*)malloc (iNumIndices * sizeof (unsigned int));

  fseek (pFile, 0, SEEK_SET);

  int vertexIndex = 0;
  int normalIndex = 0;
  int faceIndex = 0;

  while (fgets (line, 256, pFile))
  {
    if (strlen (line) == 0)
    {
      continue;
    }

    if (line[0] == 'v')
    {
      // normal
      if (line[1] == 'n')
      {
        sscanf_s (line, "vn %f %f %f", &x, &y, &z);
        pNormals[normalIndex + 0] = x;
        pNormals[normalIndex + 1] = y;
        pNormals[normalIndex + 2] = z;
        normalIndex += 3;
      }
      else
      {
        sscanf (line, "v %f %f %f %f %f %f", &x, &y, &z, &r, &g, &b);
        pVertices[vertexIndex + 0] = x/1000;
        pVertices[vertexIndex + 1] = y/1000;
        pVertices[vertexIndex + 2] = z/1000;
        vertexIndex += 3;
      }
    }
    else if (line[0] == 'f')
    {
      sscanf (line, "f %d//%d %d//%d %d//%d", &i1, &in1, &i2, &in2, &i3, &in3);
      pIndices[faceIndex + 0] = i1 - 1;
      pIndices[faceIndex + 1] = i2 - 1;
      pIndices[faceIndex + 2] = i3 - 1;
      faceIndex += 3;
    }
  }
  fclose (pFile);

  /* normalize vertex normals */
  for (int i = 0; i<iNumVertices; i += 3)
    VEC3_NORMALIZE (pNormals + i);


  /* set attribute arrays */
  giBindAttrib (GI_POSITION_ATTRIB, 0);
  giBindAttrib (GI_PARAM_ATTRIB, 2);
  giAttribPointer (0, 3, GI_FALSE, 0, pVertices);
  giAttribPointer (1, 3, GI_TRUE, 0, pNormals);
  giEnableAttribArray (0);
  giEnableAttribArray (1);

  /* create mesh */
  uiMesh = giGenMesh ();
  giBindMesh (uiMesh);
  giGetError ();
  giIndexedMesh (0, iNumVertices - 1, iNumIndices, pIndices);

  /* clean up */
  free (pVertices);
  free (pNormals);
  free (pIndices);

  int err = giGetError ();
  if (err != GI_NO_ERROR)
    return 0;

  return uiMesh;  
}

/* read PLY2 file and create OpenGI mesh */
unsigned int create_mesh(const char *filename)
{
	unsigned int uiMesh;
	FILE *pFile = fopen(filename, "r");
	float *pVertices, *pNormals;
	unsigned int *pIndices;
	int i, j, f, iNumVertices, iNumIndices;
	unsigned int i0, i1, i2;
	float v1[3], v2[3], n[3];
	if(!pFile)
	{
		fprintf(stderr, "cannot open file!\n");
		return 0;
	}

	/* read vertex data */
	fscanf(pFile, "%d\n%d\n", &iNumVertices, &iNumIndices);
	iNumVertices *= 3;
	iNumIndices *= 3;
	pVertices = (float*)malloc(iNumVertices*sizeof(float));
	pNormals = (float*)calloc(iNumVertices, sizeof(float));
	pIndices = (unsigned int*)malloc(iNumIndices*sizeof(unsigned int));
	for(i=0; i<iNumVertices; ++i)
		fscanf(pFile, "%f\n", pVertices+i);

	/* read index data and average face normals */
	for(i=0; i<iNumIndices; i+=3)
	{
		fscanf(pFile, "%d\n", &f);
		for(j=0; j<f; ++j)
			fscanf(pFile, "%d\n", pIndices+i+j);
		i0 = 3 * pIndices[i];
		i1 = 3 * pIndices[i+1];
		i2 = 3 * pIndices[i+2];
		VEC3_SUB(v1, pVertices+i1, pVertices+i0);
		VEC3_SUB(v2, pVertices+i2, pVertices+i0);
		VEC3_CROSS(n, v1, v2);
		VEC3_ADD(pNormals+i0, pNormals+i0, n);
		VEC3_ADD(pNormals+i1, pNormals+i1, n);
		VEC3_ADD(pNormals+i2, pNormals+i2, n);
	}

	/* normalize vertex normals */
	for(i=0; i<iNumVertices; i+=3)
		VEC3_NORMALIZE(pNormals+i);

	/* set attribute arrays */
	giBindAttrib(GI_POSITION_ATTRIB, 0);
	giBindAttrib(GI_PARAM_ATTRIB, 2);
	giAttribPointer(0, 3, GI_FALSE, 0, pVertices);
	giAttribPointer(1, 3, GI_TRUE, 0, pNormals);
	giEnableAttribArray(0);
	giEnableAttribArray(1);

	/* create mesh */
	uiMesh = giGenMesh();
	giBindMesh(uiMesh);
	giGetError();
	giIndexedMesh(0, iNumVertices-1, iNumIndices, pIndices);

	/* clean up */
	free(pVertices);
	free(pNormals);
	free(pIndices);
	if(giGetError() != GI_NO_ERROR)
		return 0;
	return uiMesh;
}

/* cut mesh */
int cut_mesh(unsigned int mesh)
{
	GIboolean bCut;

	/* cut mesh */ 
	giBindMesh(mesh);
	giCutterParameteri(GI_CUTTER, GI_INITIAL_GIM);
	giCut();

	/* check success */
	giGetMeshbv(GI_HAS_CUT, &bCut);
	if(!bCut)
		return -1;
	return 0;
}

/* parameterize mesh */
int parameterize_mesh(unsigned int mesh, int res)
{
	GIboolean bParams;

	/* configure parameterization and parameterize mesh */ 
	giBindMesh(mesh);
	giParameterizerParameteri(GI_PARAMETERIZER, GI_STRETCH_MINIMIZING);
	giParameterizerParameteri(GI_INITIAL_PARAMETERIZATION, GI_MEAN_VALUE);
	giParameterizerParameterf(GI_STRETCH_WEIGHT, 1.0f);
	giParameterizerParameteri(GI_PARAM_RESOLUTION, res);
	giParameterize();

	/* check success */
	giGetMeshbv(GI_HAS_PARAMS, &bParams);
	if(!bParams)
		return -1;
	return 0;
}

/* create geometry, normal and texture image */
int create_gim(unsigned int *gim, int res)
{
	static const double radius = 1.0;
	unsigned int uiTex[2];
	int iAttribCount, iNRes = (res&1) ? (res<<1)-1 : (res<<1);

	/* create textures to store geometry and normal data */
	glGenTextures(2, uiTex);
	glBindTexture(GL_TEXTURE_2D, uiTex[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, res, res, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, uiTex[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iNRes, iNRes, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	/* create images and specify data */
	giGenImages(3, gim);
	giBindImage(gim[0]);
	giImageGLTextureData(res, res, 4, GL_FLOAT, uiTex[0]);
	giBindImage(gim[1]);
	giImageGLTextureData(iNRes, iNRes, 3, GI_UNSIGNED_BYTE, uiTex[1]);
	giBindImage(gim[2]);
	giImageGLTextureData(256, 256, 1, GI_UNSIGNED_BYTE, uiPattern);

	/* sample into position and normal images */
	giAttribImage(0, gim[0]);
	giAttribImage(1, gim[1]);
	giAttribImage(2, 0);
	giSamplerParameteri(GI_SAMPLER, GI_SAMPLER_SOFTWARE);
	giAttribSamplerParameteri(0, GI_SAMPLING_MODE, GI_SAMPLE_DEFAULT);
	giAttribSamplerParameteri(1, GI_SAMPLING_MODE, GI_SAMPLE_NORMALIZED);
	giSample();

	/* check success */
	giGetIntegerv(GI_SAMPLED_ATTRIB_COUNT, &iAttribCount);
	if(iAttribCount != 2)
	{
		giDeleteImages(3, gim);
		return -1;
	}
	return 0;
}

/* render geometry image */
void display(void)
{
	static int iFrame = 0, iTime, iTimebase = 0;

	/* clear screen and set camera */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -fZ);
	glRotatef(fAngleX, 1.0f, 0.0f, 0.0f);
	glRotatef(fAngleY, 0.0f, 1.0f, 0.0f);

	/* set pattern texture */
	if(iParam && iDrawMesh)
	{
		glBindTexture(GL_TEXTURE_2D, uiPattern);
		glEnable(GL_TEXTURE_2D);
	}
	else if(!iDrawMesh)
		giAttribImage(2, iParam ? uiGIM[2] : 0);

	/* let OpenGI render the Mesh or the Geometry Image */
	if(iDrawMesh)
		glCallList(uiList);
	else
		giGLDrawGIM();
	if(iWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_LIGHTING);
		glColor3f(0.5f, 0.5f, 0.5f);
		if(iDrawMesh)
			glCallList(uiList);
		else
		{
			giAttribImage(1, 0);
			giGLDrawGIM();
			giAttribImage(1, uiGIM[1]);
		}
		glEnable(GL_LIGHTING);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	/* restore state */
	if(iParam && iDrawMesh)
		glDisable(GL_TEXTURE_2D);

	/* compute and display FPS */
	++iFrame;
	iTime = glutGet(GLUT_ELAPSED_TIME);
	if(iTime-iTimebase > 1000)
	{
		if(iFPS)
		{
			float fFPS = iFrame * 1000.0/(iTime-iTimebase);
			printf("FPS: %.2f\n", fFPS);
		}
		iTimebase = iTime;
		iFrame = 0;
	}
	glutSwapBuffers();
}

/* react on resizing of window */
void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (double)w/(double)h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	iWidth = w;
	iHeight = h;
}

/* keyboard event */
void keyboard(unsigned char key, int w, int h)
{
	static const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static const float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static const float amb[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
	static const float diff[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	static unsigned int uiCullFace = 0;
	GIboolean b;
	float fRadius;
	switch(key)
	{
	/* change culling mode */
	case 'c':
		if(uiCullFace == GL_BACK)
			uiCullFace = GL_FRONT;
		else if(uiCullFace == GL_FRONT)
			uiCullFace = 0;
		else
			uiCullFace = GL_BACK;
		if(uiCullFace)
		{
			glCullFace(uiCullFace);
			glEnable(GL_CULL_FACE);
		}
		else
			glDisable(GL_CULL_FACE);
		printf("culling %s\n", uiCullFace ? (uiCullFace==GL_FRONT ? 
			"frontfaces" : "backfaces") : "disabled");
		break;

	/* toggle display of FPS */
	case 'f':
		iFPS = !iFPS;
		break;

	/* toggle use of geometry shader */
	case 'g':
		giGetBooleanv(GI_GL_USE_GEOMETRY_SHADER, &b);
		giGLRenderParameterb(GI_GL_USE_GEOMETRY_SHADER, !b);
		printf("geometry shader %s\n", b ? "disabled" : "enabled");
		break;

	/* switch between mesh and remesh */
	case 'm':
		iDrawMesh = !iDrawMesh;
		printf("drawing %s\n", iDrawMesh ? "mesh" : "geometry image");
		break;

	/* visualize parameterization */
	case 'p':
		iParam = !iParam;
		if(iParam)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
		}
		else
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
		}
		break;

	/* toggle use of vertex texturing */
	case 't':
		giGetBooleanv(GI_GL_USE_VERTEX_TEXTURE, &b);
		giGLRenderParameterb(GI_GL_USE_VERTEX_TEXTURE, !b);
		printf("vertex texturing %s\n", b ? "disabled" : "enabled");
		break;

	/* view all */
	case 'v':
		giBindMesh(uiMesh);
		giGetMeshfv(GI_RADIUS, &fRadius);
		fZ = fRadius * 1.9f;
		break;

	/* wireframe overlay */
	case 'w':
		iWireframe = !iWireframe;
		break;

	/* exit */
	case 27:
		exit(0);
	}
}

/* mouse click */
void mouse(int button, int state, int x, int y)
{
	iLastX = x;
	iLastY = y;
	iButton = button;
}

/* mouse move */
void motion(int x, int y)
{
	if(iButton == GLUT_LEFT_BUTTON)
	{
		fAngleX = fmod(fAngleX+360.0f*(float)(y-iLastY)/iHeight, 360.0f);
		fAngleY = fmod(fAngleY+360.0f*(float)(x-iLastX)/iWidth, 360.0f);
		iLastX = x;
		iLastY = y;
	}
	else if(iButton == GLUT_RIGHT_BUTTON)
	{
		fZ *= 1.0f - 2.0f*(float)(iLastY-y)/iHeight;
		iLastY = y;
	}
}

/* OpenGI error callback */
void GICALLBACK errorCB(unsigned int error, void *data)
{
	/* actually default callback in debug but we want it in release, too */
	fprintf(stderr, "OpenGI error: %s!\n", giErrorString(error));
}
