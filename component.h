typedef enum {state, probe, and, or, not, nand, nor, xor, xnor} Builtins;

typedef struct{
    char x, y;
} Pair;

typedef struct{
    unsigned char r, g, b;
} Color;

typedef struct{
    Pair start, size, inpSrc[32];
    Color color;
    unsigned char inpNum, outNum;
    bool inputs[32], outputs[32];
} Component;

Component GetComponent(Builtins val, Pair pos){
    Component component;
    switch(val){
        case and:
            component.inpNum = 2;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){255, 0, 0};
            break;
        case or:
            component.inpNum = 2;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){0, 255, 0};
            break;
        case not:
            component.inpNum = 1;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){100, 100, 100};
            break;
        case nand:
            component.inpNum = 2;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){0, 255, 255};
            break;
        case nor:
            component.inpNum = 2;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){255, 0, 255};
            break;
        case xor:
            component.inpNum = 2;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){255, 255, 0};
            break;
        case xnor:
            component.inpNum = 2;
            component.outNum = 1;
            component.start = pos;
            component.size  = (Pair) {3, 2};
            component.color = (Color){0, 0, 255};
            break;
        case state:
            component.inpNum = 1;
            component.outNum = 0;
            component.start = pos;
            component.size  = (Pair) {1, 1};
            component.color = (Color){255, 255, 255};
            break;
        default:
            component.inpNum = 1;
            component.outNum = 0;
            component.start = pos;
            component.size  = (Pair) {1, 1};
            component.color = (Color){255, 255, 255};
            break;
    }
    return component;
}
