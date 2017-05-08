/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser, 
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack (opcode = PUSH)
  pop: remove the top matrix on the origin stack (opcode = POP)

  move/scale/rotate: create a transformation matrix (opcode = MOVE / SCALE / ROTATE)
                     based on the provided values, then 
		     multiply the current top of the
		     origins stack by it.

  box/sphere/torus: create a solid object based on the (opcode = BOX / SPHERE / TORUS)
                    provided values. Store that in a 
		    temporary matrix, multiply it by the
		    current top of the origins stack, then
		    call draw_polygons.

  line: create a line based on the provided values. Store (opcode = LINE)
        that in a temporary matrix, multiply it by the
	current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename (opcode = SAVE)

  display: view the image live (opcode = DISPLAY)
  
  jdyrlandweaver

  %token COMMENT
%token <val> DOUBLE
%token <string> LIGHT AMBIENT
%token <string> CONSTANTS SAVE_COORDS CAMERA 
%token <string> SPHERE TORUS BOX LINE CS MESH TEXTURE
%token <string> STRING
%token <string> SET MOVE SCALE ROTATE BASENAME SAVE_KNOBS TWEEN FRAMES VARY 
%token <string> PUSH POP SAVE GENERATE_RAYFILES
%token <string> SHADING SHADING_TYPE SETKNOBS FOCAL DISPLAY WEB
%token <string> CO
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"

void my_main() {

  int i;
  struct matrix *tmp;
  struct matrix *polygons;
  struct stack *s;
  screen t;
  color g;
  printf("USING MY MAIN\n");

  s = new_stack();
  tmp = new_matrix(4, 4);
  polygons = new_matrix(4, 1000);
  clear_screen( t );
  double step = 0.1; ////////PLACEHOLDER

  for (i=0;i<lastop;i++) {  
    switch (op[i].opcode) {
      case PUSH:
        printf("PUSHED\n");
        push(s);
        break;

      case POP:
        printf("POPPED\n");
        pop(s);
        break;

      double dx, dy, dz;
      case MOVE:
        dx = op[i].op.move.d[0];
        dy = op[i].op.move.d[1];
        dz = op[i].op.move.d[2];
        printf("move by (%f, %f, %f)\n", dx, dy, dz);
        tmp = make_translate(dx, dy, dz);
        matrix_mult(peek(s), tmp);
        copy_matrix(tmp, peek(s));
        break;

      double sx, sy, sz;
      case SCALE:
        sx = op[i].op.scale.d[0];
        sy = op[i].op.scale.d[1];
        sz = op[i].op.scale.d[2];
        printf("scale by (%f, %f, %f)\n", sx, sy, sz);
        tmp = make_scale(sx, sy, sz);
        matrix_mult(peek(s), tmp);
        copy_matrix(tmp, peek(s));
        break;

      int axis;
      double theta;
      case ROTATE:
        axis = op[i].op.rotate.axis;
        theta = op[i].op.rotate.degrees;
        printf("Rotation: axis = %d, angle = %f\n", axis, theta);
        if(axis == 0) tmp = make_rotX(theta * (M_PI / 180));
        if(axis == 1) tmp = make_rotY(theta * (M_PI / 180));
        if(axis == 2) tmp = make_rotZ(theta * (M_PI / 180));
        matrix_mult(peek(s), tmp);
        copy_matrix(tmp, peek(s));
        break;

      double x, y, z;
      double width, height, depth;
      case BOX:
        x = op[i].op.box.d0[0]; y = op[i].op.box.d0[1]; z = op[i].op.box.d0[2];
        width = op[i].op.box.d1[0]; height = op[i].op.box.d1[1]; depth = op[i].op.box.d1[2];
        printf("Adding box: x = %f, y = %f, z = %f, width = %f, height = %f, depth = %f\n", x, y, z, width, height, depth);
        add_box(polygons, x, y, z, width, height, depth);
        matrix_mult(peek(s), polygons);
        draw_polygons(polygons, t, g);
        polygons->lastcol = 0;
        break;

      double r;
      case SPHERE:
        x = op[i].op.sphere.d[0]; y = op[i].op.sphere.d[1]; z = op[i].op.sphere.d[2];
        r = op[i].op.sphere.r;
        printf("Adding sphere: (%f, %f, %f), radius %f\n", x, y, z, r);
        add_sphere(polygons, x, y, z, r, step);

        matrix_mult(peek(s), polygons);
        draw_polygons(polygons, t, g);
        polygons->lastcol = 0;
        break;

      double r0, r1;
      case TORUS:
        printf("TORUS\n");
        x = op[i].op.torus.d[0]; y = op[i].op.torus.d[1]; z = op[i].op.torus.d[2];
        r0 = op[i].op.torus.r0; r1 = op[i].op.torus.r1;
        add_torus(polygons, x, y, z, r0, r1, step);

        matrix_mult(peek(s), polygons);
        draw_polygons(polygons, t, g);
        polygons->lastcol = 0;
        break;

      double x0, y0, z0;
      double x1, y1, z1;
      case LINE:
        x0 = op[i].op.line.p0[0]; y0 = op[i].op.line.p0[1]; z0 = op[i].op.line.p0[2];
        x1 = op[i].op.line.p1[0]; y1 = op[i].op.line.p1[1]; z1 = op[i].op.line.p1[2];
        printf("Adding line: (%f, %f, %f) -> (%f, %f, %f)\n", x0, y0, z0, x1, y1, z1);
        add_edge(polygons, x0, y0, z0, x1, y1, z1);

        matrix_mult(peek(s), tmp);
        draw_lines(tmp, t, g);
        tmp->lastcol = 0;
        break;

      char* filename;
      case SAVE:
        filename = op[i].op.save.p->name;
        printf("filename: %s\n", filename);
        save_extension(t, filename);
        break;

      case DISPLAY:
        printf("DISPLAY\n");
        display(t);
        break;
    }
  }
}
