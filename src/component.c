#include <stdio.h>
#include "component.h"

extern Component ComponentList[256];
extern unsigned char componentCount;
extern int time;

void Tick(Component *component);
void orGate(Component *component);
void andGate(Component *component);
void notGate(Component *component);
void norGate(Component *component);
void xorGate(Component *component);
void xnorGate(Component *component);
void nandGate(Component *component);
void ToggleState(Component *component);
void ToggleProbe(Component *component);

static void (*operate[g_total])(Component *component) = {ToggleState, ToggleProbe, Tick, andGate, orGate, nandGate, norGate, xorGate, xnorGate, notGate};
Operation uDefOperations[UDEF_LIMIT];
Component uDefComponents[UDEF_LIMIT];
unsigned char uDefCompoCount = g_total;

/*
 * UserMadeCompo:
 * size
 * inum
 * onum
 * width
 * type
 * depth
*/

static bool isDone[256];
static int indexList[256];

char recurseDescent(Component *compo, int ocount, int count, int index)
{
    if (compo->type == state || compo->type == clock)
    {
        return count;
    }
    char total;
    uDefOperations[uDefCompoCount - g_total].internals[ocount][count] = *compo;
    isDone[index] = true;
    for (int i = 0; i < compo->inum; i++)
    {
        uDefOperations[uDefCompoCount - g_total].internals[ocount][count].inpSrc[i].x = indexList[uDefOperations[uDefCompoCount - g_total].internals[ocount][count].inpSrc[i].x];
        if (!isDone[compo->inpSrc[i].x])
        {
            total = count + recurseDescent(compo->inputs[i], ocount, count + 1, compo->inpSrc[i].x);
        }
    }
    return total;
}

void CreateComponent()
{
    int icount = 0, ocount = 0, ccount = 0;
    for (int i = 0; i < componentCount; i++)
    {
        if (ComponentList[i].type == state || ComponentList[i].type == clock)
        {
            indexList[i] = -(icount + 2);
            icount++;
        }
        else if (ComponentList[i].type != probe)
        {
            indexList[i] = ccount;
            ccount++;
        }
    }

    for (int i = 0; i < componentCount; i++)
    {
        if (ComponentList[i].type == probe)
        {
            uDefOperations[uDefCompoCount - g_total].internalsCount[ocount] = recurseDescent(ComponentList[i].inputs[0], ocount, 0, i);
            ocount++;
            for (int j = 0; j < 256; j++)
            {
                isDone[j] = false;
            }
        }
    }

    uDefComponents[uDefCompoCount - g_total].inum = icount;
    uDefComponents[uDefCompoCount - g_total].onum = ocount;
    uDefComponents[uDefCompoCount - g_total].size = (icount > ocount) ? icount : ocount;
    uDefComponents[uDefCompoCount - g_total].width = 4;
    uDefComponents[uDefCompoCount - g_total].type = uDefCompoCount;
    uDefComponents[uDefCompoCount - g_total].depth = 0;

    uDefCompoCount++;
}

void operateInternals(Component *parent)
{
    for (int i = 0; i < parent->onum; i++)
    {
        for (int j = 0; j < uDefOperations[parent->type - g_total].internalsCount[i]; j++)
        {
            for (int k = 0; k < uDefOperations[parent->type - g_total].internals[i][j].inum; k++)
            {
                if (uDefOperations[parent->type - g_total].internals[i][j].inpSrc[k].x >= 0){
                    uDefOperations[parent->type - g_total].internals[i][j].inputs[k] = &uDefOperations[parent->type - g_total].internals[i][j];
                }
                else{
                    uDefOperations[parent->type - g_total].internals[i][j].inputs[k] = parent->inputs[uDefOperations[parent->type - g_total].internals[i][j].inpSrc[k].x * -1 - 2];
                }
            }
        }
        for (int j = 0; j < uDefOperations[parent->type - g_total].internalsCount[i]; j++)
            update(&uDefOperations[parent->type - g_total].internals[i][j]);
        parent->outputs[i] = uDefOperations[parent->type - g_total].internals[i][0].outputs[0];
    }
}

void update(Component *component)
{
    if (component->type < g_total)
    {
        operate[component->type](component);
    }
    else
    {
        operateInternals(component);
    }
}

void GetWidthHeight(int *w, int *h, Type type, int size)
{
    if (type == state || type == clock || type == probe)
    {
        *w = 1;
        *h = 1;
    }
    else if (type == g_not)
    {
        *w = 3;
        *h = 1;
    }
    else if (type < g_total)
    {
        *w = 4;
        *h = size;
    }
    else
    {
        *w = uDefComponents[type - g_total].width;
        *h = uDefComponents[type - g_total].size;
    }
}

void SetIOPos(Component *component)
{
    for (int i = component->inum; i < MAX_TERM_NUM; i++)
    {
        component->inpPos[i].y = -1;
        component->inpPos[i].x = -1;
    }
    for (int i = component->onum; i < MAX_TERM_NUM; i++)
    {
        component->outPos[i].y = -1;
        component->outPos[i].x = -1;
    }
    for (int i = 0; i < component->inum; i++)
    {
        component->inpPos[i].x = component->start.x;
        component->inpPos[i].y = (2 * component->start.y + component->size) / component->inum * (i + 1);
    }
    for (int i = 0; i < component->onum; i++)
    {
        component->outPos[i].x = component->start.x + component->width - 1;
        component->outPos[i].y = (2 * component->start.y + component->size) / component->onum * (i + 1);
    }
}

void ClearIO(Component *component)
{
    for (int i = 0; i < MAX_TERM_NUM; i++)
    {
        component->inpSrc[i] = (Pair){-1, -1};
        component->inputs[i] = false;
        component->outputs[i] = false;
    }
}

Component SingleInputBuiltin(Type type, Pair pos)
{
    Component component;
    component.size = 1;
    component.width = 1 + 2 * (type == g_not);
    component.start = pos;
    component.depth = 0;
    component.type = type;
    ClearIO(&component);
    if (type == g_not)
    {
        component.inum = 1;
        component.onum = 1;
    }
    else if (type == probe)
    {
        component.inum = 1;
        component.onum = 0;
    }
    else
    {
        component.inum = 0;
        component.onum = 1;
    }
    SetIOPos(&component);
    return component;
}

Component MultiInputBuiltin(Type type, int inpNum, Pair pos)
{
    Component component;
    component.start = pos;
    component.size = inpNum;
    component.inum = inpNum;
    component.onum = 1;
    component.width = 4;
    component.type = type;
    component.depth = 0;
    ClearIO(&component);
    SetIOPos(&component);
    return component;
}

Component UserMadeCompo(int type, Pair pos)
{
    Component component;
    component = uDefComponents[type - g_total];
    component.start = pos;
    ClearIO(&component);
    SetIOPos(&component);
    return component;
}

Component GetComponent(Type type, char inpNum, Pair pos)
{
    if (type == state || type == clock || type == g_not || type == probe)
        return SingleInputBuiltin(type, pos);
    else if (type < g_total)
        return MultiInputBuiltin(type, inpNum, pos);
    else
        return UserMadeCompo(type, pos);
}

void SetInputs(Component *component)
{
    component->depth += 1;
    for (int i = 0; i < component->inum; i++)
    {
        if (component->inpSrc[i].x != -1 && component->depth < 2)
            update(component->inputs[i]);
    }
}

void andGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0 || component ->inpSrc[i].x < -1)
        {
            component->outputs[0] = component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y];
        }
        else
        {
            component->outputs[0] = false;
            break;
        }
    }
}

void orGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0 || component ->inpSrc[i].x < -1)
        {
            component->outputs[0] = component->outputs[0] || component->inputs[i]->outputs[component->inpSrc[i].y];
        }
        else
        {
            component->outputs[0] = component->outputs[0] || false;
        }
    }
}

void nandGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[0];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0 || component ->inpSrc[i].x < -1)
        {
            component->outputs[0] = !(component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y]);
        }
        else
        {
            component->outputs[0] = true;
            break;
        }
    }
}

void norGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0 || component ->inpSrc[i].x < -1)
        {
            component->outputs[0] = !(component->outputs[0] || component->inputs[i]->outputs[component->inpSrc[i].y]);
        }
        else
        {
            component->outputs[0] = !component->outputs[0];
        }
    }
}

void xorGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i].x >= 0 || component ->inpSrc[i].x < -1)
        {
            component->outputs[0] = (component->outputs[0] && !component->inputs[i]->outputs[component->inpSrc[i].y]) ||
                                    (!component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y]);
        }
    }
}

void xnorGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i].x >= 0 || component ->inpSrc[i].x < -1)
        {
            component->outputs[0] = (!component->outputs[0] && !component->inputs[i]->outputs[component->inpSrc[i].y]) ||
                                    (component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y]);
        }
        else
        {
            component->outputs[0] = !component->outputs[0];
        }
    }
}

void notGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0].x >= 0 || component ->inpSrc[0].x < -1)
    {
        component->outputs[0] = !component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else if (component->inpSrc[0].x == -1)
    {
        component->outputs[0] = true;
    }
}

void Tick(Component *component)
{
    if (time == 0)
        component->outputs[0] = !component->outputs[0];
}

void ToggleState(Component *component)
{
    return;
}

void ToggleProbe(Component *component)
{
    SetInputs(component);
}
