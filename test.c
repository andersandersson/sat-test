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
  Vector movement;
  Vector center;
} Object;


void draw_object(Object* o, Vector v);
void draw_projection(Projection* proj, int offset);
void draw_vector(Object* o, Vector v, double r, double g, double b);


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

  new_p = (o->shape.vectors[0].x+o->center.x)*n.x + (o->shape.vectors[0].y+o->center.y)*n.y;

  start = new_p;
  end = new_p;

  for(i=0; i<o->shape.size; i++) {  
    new_p = (o->shape.vectors[i].x+o->center.x)*n.x + (o->shape.vectors[i].y+o->center.y)*n.y;

    start = min(new_p, start);
    end = max(new_p, end);

    new_p = (o->shape.vectors[i].x+o->center.x+o->movement.x)*n.x + (o->shape.vectors[i].y+o->center.y+o->movement.y)*n.y;

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

  dx = o->movement.x;
  dy = o->movement.y;

  r = sqrt(dx*dx+dy*dy);

  o->normals.vectors[o->shape.size].x = dy / r;
  o->normals.vectors[o->shape.size].y = -dx / r;  
}

Vector calculate_result(Projection* p1, Projection* p2) {
  Vector v;
  double mag = 0, mag_1, mag_2;
  
  v.x = -p1->direction.x;
  v.y = -p1->direction.y;

  mag = max(p1->start, p1->end) - min(p2->start, p2->end);

  if(mag < 0) {
    mag = 0;
  }

  v.magnitude = mag;

  return v;
}

Vector calculate_collision(Object* o1, Object* o2) {
  Projection* o1_projections;
  Projection* o2_projections;
  Vector v, r, vd;
  Vector min_v;
  Vector direction;
  double mag, a, b, B;
  int num_axis = o1->normals.size + o2->normals.size;
  int i, min_i = 0, j;
  int c = 0, found = 0;

  o1_projections = malloc(2*num_axis*sizeof(Projection));
  o2_projections = malloc(2*num_axis*sizeof(Projection));

  for(i=0; i<o1->normals.size; i++) {
    o1_projections[2*c] = calculate_projection(o1, o1->normals.vectors[i]);
    o2_projections[2*c] = calculate_projection(o2, o1->normals.vectors[i]);
    v = o1->normals.vectors[i];
    v.x *= -1;
    v.y *= -1;
    o1_projections[2*c+1] = calculate_projection(o1, v);
    o2_projections[2*c+1] = calculate_projection(o2, v);
    c++;
  }
  for(i=0; i<o2->normals.size; i++) {
    o1_projections[2*c] = calculate_projection(o1, o2->normals.vectors[i]);
    o2_projections[2*c] = calculate_projection(o2, o2->normals.vectors[i]);
    v = o2->normals.vectors[i];
    v.x *= -1;
    v.y *= -1;
    o1_projections[2*c+1] = calculate_projection(o1, v);
    o2_projections[2*c+1] = calculate_projection(o2, v);
    c++;
  }

  min_v.x = min_v.y = min_v.magnitude = 0;

  for(i=0; i<num_axis*2; i++) {    
    //draw_projection(&o1_projections[i], 0);
    //draw_projection(&o2_projections[i], 1);
    
    v = calculate_result(&o1_projections[i], &o2_projections[i]);
    
    vd.x = v.x*v.magnitude;
    vd.y = v.y*v.magnitude;
    //draw_vector(o1, vd, 1.0, 0, 0);
    
    b = v.magnitude;
    B = o1->movement.x*v.x + o1->movement.y*v.y;
    a = b/B;
    
    r.x = a*o1->movement.x;
    r.y = a*o1->movement.y;
    r.magnitude = sqrt(r.x*r.x+r.y*r.y);
    
    //draw_vector(o1, r, 1.0, 0, 1.0);
    
    if((r.magnitude < min_v.magnitude || found == 0) && a <= 0) {
      found = 1;
      min_i = i;
      min_v = r;
    }
  }
  
  v = o1->center;
  v.x += o1->movement.x;
  v.y += o1->movement.y;
  v.x += min_v.x;
  v.y += min_v.y;
  draw_object(o1, v);
  o1_projections[min_i].direction.x *= 20;
  o1_projections[min_i].direction.y *= 20;
  draw_vector(o1, o1_projections[min_i].direction, 1.0, 0.5, 0.5);
  draw_vector(o1, min_v, 0.5, 0.8, 1.0);

  free(o1_projections);
  free(o2_projections);

  return min_v;
}

void draw_vector(Object* o, Vector v, double r, double g, double b) {
  glBegin(GL_LINES);
  glColor3f(r, g, b);
  glVertex3f(o->center.x, o->center.y, 0);
  glVertex3f(o->center.x+v.x, o->center.y+v.y, 0);
  glEnd();
  glBegin(GL_QUADS);
  glVertex3f(o->center.x+v.x-4, o->center.y+v.y-4, 0);
  glVertex3f(o->center.x+v.x+4, o->center.y+v.y-4, 0);
  glVertex3f(o->center.x+v.x+4, o->center.y+v.y+4, 0);
  glVertex3f(o->center.x+v.x-4, o->center.y+v.y+4, 0);
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
  glBegin(GL_QUADS);
  glVertex3f(proj->direction.x*proj->end+offset-2, proj->direction.y*proj->end+offset-2, 0);
  glVertex3f(proj->direction.x*proj->end+offset+2, proj->direction.y*proj->end+offset-2, 0);
  glVertex3f(proj->direction.x*proj->end+offset+2, proj->direction.y*proj->end+offset+2, 0);
  glVertex3f(proj->direction.x*proj->end+offset-2, proj->direction.y*proj->end+offset+2, 0);
  glEnd();
}

void draw_object(Object* o, Vector point) {
  int i, j;
  double x, y, k;
  Vector d;

  glBegin(GL_LINES);
  for(i=0; i<o->shape.size; i++) {
    glColor3f(o->color.r/255.0, o->color.g/255.0, o->color.b/255.0);
    j = (1+i)%o->shape.size;
    glVertex3f(o->shape.vectors[i].x+point.x, o->shape.vectors[i].y+point.y, 0);
    glVertex3f(o->shape.vectors[j].x+point.x, o->shape.vectors[j].y+point.y, 0);
  }

  glEnd();
}

Object create_object(void) {
  Object o;
  int i;
  int r = 80;
  int size;
  double x, y;


  size = 5;
  o.color.r = 128;
  o.color.g = 200;
  o.color.b = 12;

  o.shape = create_vectorgroup(size);
  o.normals = create_vectorgroup(size+1);

  x = rand() % 500 - 250;
  y = rand() % 350 - 175;

  o.center.x = x;
  o.center.y = y;

  for(i=0; i<size; i++) {
    o.shape.vectors[i].x = r*cos(2*M_PI*i/size);
    o.shape.vectors[i].y = r*sin(2*M_PI*i/size);
  }

  o.movement.x = 0;
  o.movement.y = 0;

  calculate_normals(&o);

  return o;
}

void free_object(Object o) {
  free_vectorgroup(o.shape);
  free_vectorgroup(o.normals);
}

void move_object(Object* o, double x, double y) {
  o->center.x += x; 
  o->center.y += y;
}

int main( void )
{
  double dx, dy, ox, oy, a;
  int i,j,x,y;
  int running = GL_TRUE;
  int num_obj = 2;
  int r_lock = 0, tab_lock = 0;
  int curr_index = 0;
  Object o[3], *curr_o;
  Object shade;
  Projection proj;
  Vector v;
  Vector old_pos;

  srand((unsigned)time(NULL));

  for(i=0; i<num_obj; i++) {
    o[i] = create_object();
  }

  shade = create_object();

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

      glfwGetMousePos(&x, &y);

      curr_o->movement.x = x - curr_o->center.x - 320;
      curr_o->movement.y = y - curr_o->center.y - 240;

      draw_vector(curr_o, curr_o->movement, 0, 1.0, 0);

      for(j=0; j<num_obj; j++) {
	if(&o[j] != curr_o) {
	  v = calculate_collision(curr_o, &o[j]);
	  //move_object(curr_o, v.x, v.y);
	  draw_object(&o[j], o[j].center);
	}
      }

      draw_object(curr_o, curr_o->center);
      
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
	curr_o->movement.x = curr_o->movement.y = 0;
	curr_index = (++curr_index) % num_obj;
	curr_o = &o[curr_index];
	curr_o->color.b = 255;
	move_object(curr_o, 0, 0);
	tab_lock = 1;
      }
      if(glfwGetKey(GLFW_KEY_TAB) == GLFW_RELEASE && 1 == tab_lock) {
	tab_lock = 0;
      }

      if(glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
	move_object(curr_o, curr_o->movement.x, curr_o->movement.y);
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
