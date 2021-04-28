#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"

int main(int argc, char* argv[]) {
    char ofname[128];
    if (argc < 2) { 
        fprintf(stderr,"usage: %s <radius> <norm>.\n",argv[0]); 
        return RESULT_ERROR; 
    }
    const int radius = atoi(argv[1]);    
    const int norm = atoi(argv[2]);    
    template_t* tpl;
    tpl = generate_ball_template(radius,norm,1);
    print_template(tpl);
    return 0;
}
