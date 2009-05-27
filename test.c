#include <GL/glfw.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

inline double min(double l, double r) {
  return l < r ? l : r;
}

inline double max(double l, double r) {
  return l > r ? l : r;
}

typedef struct Color {
  int r;
  int g;
  int b;
} Color;

typedef struct Vector {
  double x;
  double y;
  double magnitude;
} Vector;

typedef struct Projection {
  Vector direction;
  double start;
  double end;
} Projection;

typedef struct VectorGroup {
  int size;
  Vector* vectors;
} VectorGroup;

typedef struct Object {
  Color color;
  VectorGroup shape;
  VectorGroup normals;  
} Object;


void draw_projection(Projection* proj, int offset);
void draw_vector(Object* o, Vector v);


VectorGroup create_vectorgroup(int size) {
  int i;
  VectorGroup vg;
  
  vg.vectors = malloc(sizeof(Vector)*size);
  vg.size = size;

  return vg;
}

void free_vectorgroup(VectorGroup vg) {
  free(vg.vectors);
}

Projection calculate_projection(Object* o, Vector n) {
  double start, end, new_p;
  Projection projection;
  int i = 0;

  new_p = o->shape.vectors[0].x*n.x + o->shape.vectors[0].y*n.y;

  start = new_p;
  end = new_p;

  for(i=0; i<o->shape.size; i++) {  
    new_p = o->shape.vectors[i].x*n.x + o->shape.vectors[i].y*n.y;

    start = min(new_p, start);
    end = max(new_p, end);
  }

  projection.direction.x = n.x;
  projection.direction.y = n.y;

  projection.start = start;
  projection.end = end;

  return projection;
}


void calculate_normals(Object* o) {
  double dx, dy, r;
  int i;

  for(i=0; i<o->shape.size; i++) {
    dx = o->shape.vectors[(i+1)%o->shape.size].x - o->shape.vectors[i].x;
    dy = o->shape.vectors[(i+1)%o->shape.size].y - o->shape.vectors[i].y;

    r = sqrt(dx*dx+dy*dy);

    o->normals.vectors[i].x = dy / r;
    o->normals.vectors[i].y = -dx / r;
  }
}

Vector calculate_result(Projection* p1, Projection* p2) {
  Vector v;
  double mag = 0;
  Projection* temp;
  
  v.x = -p1->direction.x;
  v.y = -p1->direction.y;

  if(p1->start > p2->start) {
    v.x *= -1.0;
    v.y *= -1.0;
    temp = p1;
    p1 = p2;
    p2 = temp;
  }
  
  if(p1->end < p2->start) {
    mag = 0;
  } else if(p1->end < p2->end) {
    mag = p1->end - p2->start;
  } else {
    mag = p1->end - p2->start;
  }

  v.x *= mag;
  v.y *= mag;
  v.magnitude = mag;

  return v;
}

Vector calculate_collision(Object* o1, Object* o2) {
  Projection* o1_projections;
  Projection* o2_projections;
  Vector v;
  Vector min_v;
  int num_axis = o1->normals.size + o2->normals.size;
  int i;
  int c = 0;

  o1_projections = malloc(num_axis*sizeof(Projection));
  o2_projections = malloc(num_axis*sizeof(Projection));

  for(i=0; i<o1->normals.size; i++) {
    o1_projections[c] = calculate_projection(o1, o1->normals.vectors[i]);
    o2_projections[c] = calculate_projection(o2, o1->normals.vectors[i]);
    c++;
  }
  for(i=0; i<o2->normals.size; i++) {
    o1_projections[c] = calculate_projection(o1, o2->normals.vectors[i]);
    o2_projections[c] = calculate_projection(o2, o2->normals.vectors[i]);
    c++;
  }

  min_v = calculate_result(&o1_projections[0], &o2_projections[0]);
  for(i=0; i<num_axis; i++) {    
    //draw_projection(&o1_projections[i], 0);
    //draw_projection(&o2_projections[i], 1);

    v = calculate_result(&o1_projections[i], &o2_projections[i]);

    if(v.magnitude < min_v.magnitude) {
      min_v = v;
    }
  }

  //draw_vector(o1, min_v);

  free(o1_projections);
  free(o2_projections);

  return min_v;
}

void draw_vector(Object* o, Vector v) {
  int i;
  double ox, oy;
  ox = oy = 0;
  
  for(i=0; i<o->shape.size; i++) {
    ox += o->shape.vectors[i].x;
    oy += o->shape.vectors[i].y;
  }
  
  ox /= o->shape.size;
  oy /= o->shape.size;

  glBegin(GL_LINES);
  glColor3f(1.0, 0, 0.5);
  glVertex3f(ox, oy, 0);
  glVertex3f(ox+v.x, oy+v.y, 0);
  glEnd();
}

void draw_axis(Object* o) {
  int i;
  glBegin(GL_LINES);
  for(i=0; i<o->normals.size; i++) {
    glColor3f(o->color.r/255.0, o->color.g/255.0, o->color.b/255.0);
    glVertex3f(o->normals.vectors[i].x*-400, o->normals.vectors[i].y*-400, 0);
    glVertex3f(o->normals.vectors[i].x*400, o->normals.vectors[i].y*400, 0);
  }
  glEnd();
}

void draw_projection(Projection* proj, int offset) {
  glBegin(GL_LINES);
  glColor3f(0, offset, 1.0);
  glVertex3f(proj->direction.x*proj->start+offset, proj->direction.y*proj->start+offset, 0);
  glVertex3f(proj->direction.x*proj->end+offset, proj->direction.y*proj->end+offset, 0);
  glEnd();
}

void draw_object(Object* o, int draw_axis) {
  int i, j;
  double x, y, k;
  Vector d;

  glBegin(GL_LINES);
  for(i=0; i<o->shape.size; i++) {
    glColor3f(o->color.r/255.0, o->color.g/255.0, o->color.b/255.0);
    j = (1+i)%o->shape.size;
    glVertex3f(o->shape.vectors[i].x, o->shape.vectors[i].y, 0);
    glVertex3f(o->shape.vectors[j].x, o->shape.vectors[j].y, 0);
  }
  glEnd();
}

Object create_object(void) {
  Object o;
  int i;
  int r = 80;
  int size;
  double x, y;


  size = 3 + rand() % 6;
  o.color.r = 128;
  o.color.g = 200;
  o.color.b = 12;

  o.shape = create_vectorgroup(size);
  o.normals = create_vectorgroup(size);

  x = rand() % 500 - 250;
  y = rand() % 350 - 175;

  for(i=0; i<size; i++) {
    o.shape.vectors[i].x = x + r*cos(2*M_PI*i/size);
    o.shape.vectors[i].y = y + r*sin(2*M_PI*i/size);
  }

  calculate_normals(&o);

  return o;
}

void free_object(Object o) {
  free_vectorgroup(o.shape);
  free_vectorgroup(o.normals);
}

void move_object(Object* o, double x, double y) {
  int i;

  for(i=0; i<o->shape.size; i++) {
    o->shape.vectors[i].x += x;
    o->shape.vectors[i].y += y;
  }
}

int main( void )
{
  double dx, dy, ox, oy, a;
  int i,j;
  int running = GL_TRUE;
  int num_obj = 3;
  int r_lock = 0, tab_lock = 0;
  int curr_index = 0;
  Object o[3], *curr_o;
  Projection proj;
  Vector v;

  srand((unsigned)time(NULL));

  for(i=0; i<num_obj; i++) {
    o[i] = create_object();
  }

  glfwInit();
  
  if( !glfwOpenWindow( 640,480, 0,0,0,0,0,0, GLFW_WINDOW ) )
    {
        glfwTerminate();
        return 0;
    }
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();             
    glOrtho(-320, 320, 240, -240, -1, 1);
    glMatrixMode( GL_MODELVIEW );         
    glLoadIdentity();                     
    glClearColor(0,0,0,1);                
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curr_o = &o[0];

    curr_o->color.b = 255;

    while( running )
    {
      
      glClear( GL_COLOR_BUFFER_BIT );

      for(i=0; i<num_obj; i++) {
	for(j=0; j<num_obj; j++) {
	  if(i != j && &o[i] != curr_o) {
	    v = calculate_collision(&o[i], &o[j]);      
	    move_object(&o[i], v.x, v.y);	  
	  }
	}
	draw_object(&o[i], 0);
      }
      
      glfwSwapBuffers();

      if(glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS) {
	move_object(curr_o, -0.03, 0);
      }

      if(glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS) {
	move_object(curr_o, 0.03, 0);
      }

      if(glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS) {
	move_object(curr_o, 0, -0.03);
      }

      if(glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS) {
	move_object(curr_o, 0, 0.03);
      }

      if(glfwGetKey('A') == GLFW_PRESS) {
	ox = oy = 0;

	for(i=0; i<curr_o->shape.size; i++) {
	  ox += curr_o->shape.vectors[i].x;
	  oy += curr_o->shape.vectors[i].y;
	}

	ox /= curr_o->shape.size;
	oy /= curr_o->shape.size;
	
	for(i=0; i<curr_o->shape.size; i++) {	  
	  dx = curr_o->shape.vectors[i].x - ox;
	  dy = curr_o->shape.vectors[i].y - oy;
	
	  a = 0.0002;
	  curr_o->shape.vectors[i].x = ox + cos(a)*dx - sin(a)*dy;
	  curr_o->shape.vectors[i].y = oy + cos(a)*dy + sin(a)*dx;
	}
      }
     
      if(glfwGetKey('Z') == GLFW_PRESS) {
	ox = oy = 0;

	for(i=0; i<curr_o->shape.size; i++) {
	  ox += curr_o->shape.vectors[i].x;
	  oy += curr_o->shape.vectors[i].y;
	}

	ox /= curr_o->shape.size;
	oy /= curr_o->shape.size;
	
	for(i=0; i<curr_o->shape.size; i++) {	  
	  dx = curr_o->shape.vectors[i].x - ox;
	  dy = curr_o->shape.vectors[i].y - oy;
	
	  a = -0.0002;
	  curr_o->shape.vectors[i].x = ox + cos(a)*dx - sin(a)*dy;
	  curr_o->shape.vectors[i].y = oy + cos(a)*dy + sin(a)*dx;
	}
      }

      if(glfwGetKey('R') == GLFW_PRESS && 0 == r_lock) {
	for(i=0; i<num_obj; i++) {
	  free_object(o[i]);
	}
	for(i=0; i<num_obj; i++) {
	  o[i] = create_object();
	}
	r_lock = 1;
      }
      if(glfwGetKey('R') == GLFW_RELEASE && 1 == r_lock) {
	r_lock = 0;	
      }
     
      if(glfwGetKey(GLFW_KEY_TAB) == GLFW_PRESS && 0 == tab_lock) {
	curr_o->color.b = 12;
	curr_index = (++curr_index) % num_obj;
	curr_o = &o[curr_index];
	curr_o->color.b = 255;
	tab_lock = 1;
      }
      if(glfwGetKey(GLFW_KEY_TAB) == GLFW_RELEASE && 1 == tab_lock) {
	tab_lock = 0;
      }
     

      for(i=0; i<num_obj; i++) {
	calculate_normals(&o[i]);
      }

      running = !glfwGetKey( GLFW_KEY_ESC ) &&
	glfwGetWindowParam( GLFW_OPENED );
    }

    for(i=0; i<num_obj; i++) {
      free_object(o[i]);
    }

    glfwTerminate();
    return 0;
}
