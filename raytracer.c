//Raytraces 5 spheres on a green plane with a yellow background.

#include <math.h>
#include <stddef.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


// N is the height of the bitmap; M is its width
#define N 600 
#define M 600 
// height and width of picture
#define HEIGHT 600
#define WIDTH 600
#define BIG 9999999999

struct ray {
  GLfloat point[3];
  GLfloat direction[3];
};

struct intersection {
  GLfloat point[3];
  GLfloat normal[3];
  int objectNumber;
};

typedef struct object {
  int objNum; //objects 0 and 1 are spheres, 2 is the plane
  GLfloat size; //for spheres, size is radius
  GLfloat color[3];
  GLfloat Ka;
  GLfloat Kd;
  GLfloat Ks;
  GLfloat center[3];
} object;

//big red sphere
object sphere0 = {
  0, 1, {1.0, 0.0, 0.0},
  .2, .4, 0.8,
  {-.2, 0.0, 1} };

//medium yellow sphere
object sphere1 = {
  1, .8, {1, 1, 0},
  .2, .45, .7,
  {.8, 1.8, .8} };

//small green sphere
object sphere2 = {
  2, .5, {0, 1, 0},
  .2, .45, 0.5,
  {5.8, .4, .5} };

//medium blue sphere
object sphere3 = {
  3, .6, {0, 0, 1},
  .2, .45, 0.5,
  {3.8, -1.3, .6} };

//green plane where sphere sit
object plane = {
  4, 0, {0.0, 1.0, 1.0},
  .3, .5, .2, 
  {0,0,0} };

//struct to define light's properties
typedef struct lightingStruct {
  GLfloat color[3];
  GLfloat position[3];
} lightingStruct;

lightingStruct light0 = {
  {1.0, 1.0, 1.0},
  {45.0, -3, 20.0} }; 

//white light above the viewer
lightingStruct light1 = {
  {1.0, 1.0, 1.0},
  {45, -.6, 2.2} };

GLfloat ViewerPosition[3] = {45, -.6, 2.2};
GLfloat GridX = 10, GridY = -2, GridZ = 3; //Upper left corner pixel grid 
GLfloat GridWidth = 4, GridHeight = 4;  //dimensions of the pixel grid. 

GLfloat image[N][M][3];			//image bitmap
GLfloat BLACK[3] = {0.0, 0.0, 0.0};
GLfloat background[3] = {.7, .7, 0};    //yellow background color

//maxLevel and minWeight help determine when recursive trace method ends
int maxLevel = 4;
GLfloat minWeight = 0.1f;

void init() {
  glClearColor(1.0, 1.0, 0.0, 0.0);  // yellow background
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, 600.0, 0.0, 600.0 );
}

void reshape(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, (GLfloat)w, 0.0, (GLfloat)h );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, w, h);
}


void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(300-M/2, 300-N/2);  // position of the lower left corner
                                    // of the bitmap in window coords
  glDrawPixels(M, N, GL_RGB, GL_FLOAT, image);  
  glFlush();
}

void copy(GLfloat *x, GLfloat *y) {
  /* sets x = y */
  int i;
  for (i = 0; i < 3; i++) {
    x[i] = y[i];
  }
}

//returns the dot product of two vectors
GLfloat dotProduct(GLfloat *vec1, GLfloat *vec2) {
  return (vec1[0]*vec2[0]+vec1[1]*vec2[1]+vec1[2]*vec2[2]);
}

//makes a ray from specified location to specified location
void MakeRay(GLfloat *towards, GLfloat *from, struct ray *r) {
  GLfloat direction[3];
  copy(r->point, from);
  int i;
  for (i = 0; i < 3; i++) {
    direction[i] = towards[i] - from[i]; 
  }
  GLfloat length = sqrtf(powf(direction[0],2)+powf(direction[1],2)+powf(direction[2],2));
  GLfloat normalize[3] = {direction[0]/length,direction[1]/length,direction[2]/length};
  copy(r->direction, normalize);
}

//makes the ray from the viewer through pixel (i,j)
void MakePixelRay(int i, int j, struct ray *r) {
  GLfloat direction[3];
  GLfloat pixel[3]; // The world coordinates of the (i, j) pixel
  int k;
  copy(r->point, ViewerPosition);
  pixel[0] = GridX;
  pixel[1] = GridY + (GridWidth*j)/M;
  pixel[2] = GridZ - (GridHeight*i)/N;
  for (k = 0; k < 3; k++ ) {
    direction[k] = pixel[k] - r->point[k];
  }
  GLfloat length = sqrtf(powf(direction[0],2)+powf(direction[1],2)+powf(direction[2],2));
  GLfloat normalize[3] = {direction[0]/length,direction[1]/length,direction[2]/length};
  copy(r->direction, normalize);
}

// Method inputs a ray and a sphere and returns the smallest 
// value of t at which the ray intersects this sphere  
GLfloat intersectSphere(struct ray *r, struct object *s) {
  GLfloat t1,t2;
  //compute a, b and c values for use in quadratic formula
  GLfloat a = powf(r->direction[0],2)+powf(r->direction[1],2)+powf(r->direction[2],2);
  GLfloat b = 2*((r->direction[0])*((r->point[0])-(s->center[0]))+(r->direction[1])*
    ((r->point[1])-(s->center[1]))+(r->direction[2])*((r->point[2])-(s->center[2])));
  GLfloat c = powf(s->center[0],2)+powf(s->center[1],2)+powf(s->center[2],2)+
    powf(r->point[0],2)+powf(r->point[1],2)+powf(r->point[2],2)-2*((s->center[0])*
    (r->point[0])+(s->center[1])*(r->point[1])+(s->center[2]*(r->point[2])))-powf(s->size,2);
  //if discriminant is negative, not a valid solution, so return -1
  if (powf(b,2)-4*a*c < 0)
    return -1.0;
  t1 = (-b+sqrtf(powf(b,2)-4*a*c))/2*a;
  t2 = (-b-sqrtf(powf(b,2)-4*a*c))/2*a;
  //return minimum of t values
  return fmin(t1, t2);
}

// Method inputs a ray and a sphere and returns the t value of intersection
GLfloat intersectPlane(struct ray *r, struct object *s) {
  return ((s->center[2])-(r->point[2]))/r->direction[2];
}

// Global Intersect method takes a ray, calls each 
// of the object's intersect methods, and returns the 
// intersection point and normal vector for the nearest 
// object hit by the ray (the object with the smallest 
// t-value for it's intersection) if the ray intersects 
// any of the objects in the scene, and null otherwise 
struct intersection *Intersect(struct ray *r) {
  struct intersection *data;
  data = malloc(sizeof(struct intersection));
  struct object object;
  //call intersect method of each object in the scene to be rendered
  GLfloat t0 = intersectSphere(r, &sphere0);
  GLfloat t1 = intersectSphere(r, &sphere1);
  GLfloat t2 = intersectSphere(r, &sphere2);
  GLfloat t3 = intersectSphere(r, &sphere3);
  GLfloat t4 = intersectPlane(r, &plane);
  //array to hold minimum t values
  GLfloat t_arr[5] = {t0,t1,t2,t3,t4};
  GLfloat t;
  int i;
  //loop through t values: if any are smaller than epsilon (.003),
  //set to a huge number so that the object corresponding to this value
  //won't be chosen as the nearest object
  for (i = 0; i < 5; i++) {
    if (t_arr[i] <= 0.003) 
      t_arr[i] = BIG;
  }
  //find minimum t value and set corresponding members of data struct 
  //to the appropriate values
  t = fmin(t_arr[0], fmin(t_arr[1], fmin(t_arr[2], fmin(t_arr[3], t_arr[4]))));
  if (t == t0) {
    data->objectNumber = 0;
    object = sphere0;
  }
  if (t == t1) {
    data->objectNumber = 1;
    object = sphere1;
  }
  if (t == t2) {
    data->objectNumber = 2;
    object = sphere2;
  }
  if (t == t3) {
    data->objectNumber = 3;
    object = sphere3;
  }
  if (t == t4) {
    data->objectNumber = 4;
    object = plane;
  }
  //if there is an positive intersection with an object
  if (t != BIG) { 
    data->point[0] = r->point[0] + t*(r->direction[0]);
    data->point[1] = r->point[1] + t*(r->direction[1]);
    data->point[2] = r->point[2] + t*(r->direction[2]);
    //if the nearest object is a plane, normal is (0,0,1)
    if (t == t4) {
      data->normal[0] = 0;
      data->normal[1] = 0;
      data->normal[2] = 1;  
    } 
    //otherwise, if object is a sphere, do normal calculation
    else {
      data->normal[0] = (data->point[0]-object.center[0])/object.size;
      data->normal[1] = (data->point[1]-object.center[1])/object.size;
      data->normal[2] = (data->point[2]-object.center[2])/object.size;
    }
   return data;
  }
  //if there are no positive intersections with any objects, return null
  else
    return NULL;
}

// Recursive trace method
// Arguments:
//	Ray: ray being traced
//	level: counter for how deeply you are buried in the recursion
//	weight: accumulated constant of light reflection
// Returns: the color of the ray being traced
GLfloat *Trace(struct ray *ray, int level, float weight) {
  GLfloat *color = malloc(3*sizeof(GLfloat));
  if (level > maxLevel) 
     copy(color, BLACK);
  else {
    struct intersection *p;
    p = Intersect(ray);
    if (p != NULL) {
      struct object object;
      //determine object intersected by object number
      if (p->objectNumber == 0) 
        object = sphere0;
      else if (p->objectNumber == 1) 
        object = sphere1;
      else if (p->objectNumber == 2)
        object = sphere2;
      else if (p->objectNumber == 3)
        object = sphere3;
      else if (p->objectNumber == 4)
 	object = plane;
      //ambient calculation
      GLfloat r = background[0]*(object.color[0]);
      GLfloat g = background[1]*(object.color[1]);
      GLfloat b = background[2]*(object.color[2]);      
      GLfloat ambient[3] = {r,g,b};
      struct ray *rObjLight = malloc(sizeof(struct ray));
       GLfloat *specular = malloc(3*sizeof(GLfloat));
      struct lightingStruct light;
      GLfloat diffuse[3] = {0,0,0};
      //iterate through all lights in the scene for diffuse calculation
      int k;
      for (k = 0; k < 2; k++) {
        if (k == 0) 
  	  light = light0; 
 	else
 	  light = light1;     
        //create a ray from intersected object towards light source
        MakeRay(light.position,p->point,rObjLight);
        struct intersection *i;
        i = Intersect(rObjLight);
        GLfloat dot = dotProduct(p->normal,rObjLight->direction);
        //if ray reaches light source without hitting an object, 
        //the object is not in shadow, so do a diffuse calculation
        if (i =! NULL) {
	  r = dot*light.color[0]*object.color[0];
          g = dot*light.color[1]*object.color[1];
    	  b = dot*light.color[2]*object.color[2];
 	  //if r, g, or b values are too high, set them to 1 so the
	  //object isn't washed out
          if (r > 1) 
	    r = 1;
          if (g > 1)
	    g = 1;
          if (b > 1)
            b = 1;
          diffuse[0] += r;
          diffuse[1] += g;
          diffuse[2] += b;
        }
        //specular calculation -- reflections
        struct ray *reflected = malloc(sizeof(struct ray));
        //reflected ray originiates from intersection point on object
        copy(reflected->point, p->point);
        //determine direction of reflected ray
        reflected->direction[0] = p->normal[0]*2*dot-rObjLight->direction[0];
        reflected->direction[1] = p->normal[1]*2*dot-rObjLight->direction[1];
        reflected->direction[2] = p->normal[2]*2*dot-rObjLight->direction[2];
        weight *= object.Ks;
        //condition to break from recursion (if weight < minWeight)
        if (weight >= minWeight) {
          //recursive on the reflected ray. value returned is specular color at that point
          color = Trace(reflected, level+1, weight);
          copy(specular,color);
        } 
      }
      //sum ambient, diffuse, and specular components to determine color of the object at this point
      color[0] = (object.Ka)*ambient[0]+(object.Kd)*diffuse[0]+(object.Ks)*specular[0];
      color[1] = (object.Ka)*ambient[1]+(object.Kd)*diffuse[1]+(object.Ks)*specular[1];
      color[2] = (object.Ka)*ambient[2]+(object.Kd)*diffuse[2]+(object.Ks)*specular[2];
   }  
   else {
      //ray has missed the objects, so color background color
      if (level == 0) 
        copy(color,background);
      else if (level > 0) 
	copy(color, BLACK);
    }
  }
  return color;
}

void MakePicture() {  
  // This runs through the pixel grid, makes a ray from the
  // viewer through the pixel, and traces this ray.
  // The pixel gets the color returned by the trace.
  struct ray r;
  int i, j;
  GLfloat *color;
  for (i =0; i < N; i++) {
    for (j = 0; j < M; j++ ) {
      MakePixelRay(i, j, &r);
      color = Trace(&r, 0, 1);
      copy(image[N-i-1][j], color);
     }
  }
}

int main(int argc, char** argv) {
  MakePicture();
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  glutInitWindowSize(WIDTH, HEIGHT);
  glutInitWindowPosition(50, 50);
  glutCreateWindow("RAYTRACER");
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  init();
  glutMainLoop();
}
