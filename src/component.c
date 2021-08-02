#include <stdio.h>
#include "settings.h"
#include "component.h"

extern Component ComponentList[256];
extern int time;
extern bool AlreadyUpdated[256];

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

void SetInputs(Component *component)
{
    component->depth += 1;
    for (int i = 0; i < component->inum; i++)
    {
        if (component->inpSrc[i].x != -1 && component->depth < 2)
            if (! AlreadyUpdated[component->inpSrc[i].x]){
                update(component->inputs[i]);
                AlreadyUpdated[component->inpSrc[i].x] = true;
            }
    }
}

void update(Component *component)
{
    if (component-> type != clock)
        SetInputs(component);
    operate[component->type](component);
}

void GetWidthHeight(int *w, int *h, Type type, int size)
{
    if (type == state || type == clock || type == probe)
    {
        *w = SCALE;
        *h = SCALE;
    }
    else if (type == g_not)
    {
        *w = 3 * SCALE;
        *h = SCALE;
    }
    else
    {
        *w = 4 * SCALE;
        *h = size * SCALE;
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
    component.size *= SCALE;
    component.width *= SCALE;
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
    component.size *= SCALE;
    component.width *= SCALE;
    component.type = type;
    component.depth = 0;
    ClearIO(&component);
    SetIOPos(&component);
    return component;
}

Component GetComponent(Type type, char inpNum, Pair pos)
{
    if (type == state || type == clock || type == g_not || type == probe)
        return SingleInputBuiltin(type, pos);
    else
        return MultiInputBuiltin(type, inpNum, pos);
}

void andGate(Component *component)
{
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
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
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
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
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[0];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
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
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
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
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
        {
            component->outputs[0] = (component->outputs[0] && !component->inputs[i]->outputs[component->inpSrc[i].y]) ||
                                (!component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y]);
        }
    }
}

void xnorGate(Component *component)
{
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
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
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = !component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
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
{ return; }

void ToggleProbe(Component *component){};
