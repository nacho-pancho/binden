#include <stdio.h>
#include <stdlib.h>

#include "pnm.h"
#include "image.h"
#include "templates.h"

int main(int argc, char* argv[]) {
    if (argc < 2) { 
        fprintf(stderr,"usage: %s <template>.\n",argv[1]); 
        return RESULT_ERROR; 
    }
    patch_template_t* tpl = read_template(argv[1]);
    if (tpl->k == 0) {
        fprintf(stderr,"Could not read template from %s\n",argv[1]);
        return RESULT_ERROR;
    }
    printf("Loaded template\n");
    print_template(tpl);
    printf("Sorted template\n");
    sort_template(tpl,1); // sorted by norm
    print_template(tpl);
    free_patch_template(tpl);
    return 0;
}
