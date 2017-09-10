#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"

void createDotFromAddg( ADDG* addg, char* filename )
{
  FILE *fp;
  unsigned int i, j;

  if ((fp = fopen(filename, "w")) == NULL) {
    printf("Cannot open file %s for writing into dot file\nExiting system\n", filename);
    exit(0);
  }
  
  fprintf(fp, "digraph G {\n");
  for (i = 0; i < addg->numVertices; i++) {
    if (addg->vertex[i]->type == 0) {
      fprintf(fp, "  %s [shape=box,label=\"%s\"];\n", addg->vertex[i]->name, addg->vertex[i]->name);
    } else {
      fprintf(fp, "  %s [shape=ellipse,label=\"%s\"];\n", addg->vertex[i]->name, addg->vertex[i]->name);
    }
  }

  for (i = 0; i < addg->numVertices; i++) {
    for (j = 0; j < addg->vertex[i]->outDegree; j++) {
      fprintf(fp, "  %s -> %s;\n", addg->vertex[i]->name, addg->vertex[i]->child[j]->name);
    }
  }
  fprintf(fp, "}\n");

  fclose(fp); 
}
