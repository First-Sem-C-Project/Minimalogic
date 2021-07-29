#include <stdio.h>
#include "component.h"

extern Component ComponentList[256];
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

void update(Component *component)
{
    operate[component->type](component);
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
    else
    {
        *w = 4;
        *h = size;
    }
}

void SetIOPos(Component *component, int inpNum)
{
    for (int i = inpNum; i < MAX_INPUT_NUM; i++)
    {
        component->inpPos[i].y = -1;
        component->inpPos[i].x = -1;
    }
    for (int i = 0; i < inpNum; i++)
    {
        component->inpPos[i].y = component->start.y + i;
        component->inpPos[i].x = component->start.x;
    }
    component->outPos.x = component->start.x + component->width - 1;
    component->outPos.y = component->start.y + component->size / 2;
    if (component->type == probe)
    {
        component->outPos.x = -1;
        component->outPos.y = -1;
    }
}

void ClearIO(Component *component)
{
    for (int i = 0; i < MAX_INPUT_NUM; i++)
    {
        component->inpSrc[i] = -1;
        component->inputs[i] = false;
    }
    component->output = false;
}

Component MakeSingleInputCompo(Type type, Pair pos)
{
    Component component;
    component.size = 1;
    component.width = 1 + 2 * (type == g_not);
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.depth = 0;
    component.type = type;
    ClearIO(&component);
    if (type == g_not || type == probe)
        SetIOPos(&component, 1);
    else
        SetIOPos(&component, 0);
    return component;
}

Component MakeMultiInputCompo(Type type, int inpNum, Pair pos)
{
    Component component;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.size = inpNum;
    component.width = 4;
    component.type = type;
    component.depth = 0;
    ClearIO(&component);
    SetIOPos(&component, inpNum);
    return component;
}

Component GetComponent(Type type, char inpNum, Pair pos)
{
    if (type == state || type == clock || type == g_not || type == probe)
        return MakeSingleInputCompo(type, pos);
    else
        return MakeMultiInputCompo(type, inpNum, pos);
}

void SetInputs(Component *component)
{
    component->depth += 1;
    for (int i = 0; i < component->size; i++)
    {
        if (component->inpSrc[i] != -1 && component->depth < 2)
            update(component->inputs[i]);
    }
}

void andGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = component->inputs[0]->output;
    }
    else
    {
        component->output = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i] >= 0)
        {
            component->output = component->output && component->inputs[i]->output;
        }
        else
        {
            component->output = false;
            break;
        }
    }
}

void orGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = component->inputs[0]->output;
    }
    else
    {
        component->output = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i] >= 0)
        {
            component->output = component->output || component->inputs[i]->output;
        }
        else
        {
            component->output = component->output || false;
        }
    }
}

void nandGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = component->inputs[0]->output;
    }
    else
    {
        component->output = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i] >= 0)
        {
            component->output = !(component->output && component->inputs[i]->output);
        }
        else
        {
            component->output = true;
            break;
        }
    }
}

void norGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = component->inputs[0]->output;
    }
    else
    {
        component->output = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i] >= 0)
        {
            component->output = !(component->output || component->inputs[i]->output);
        }
        else
        {
            component->output = !component->output;
        }
    }
}

void xorGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = component->inputs[0]->output;
    }
    else
    {
        component->output = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i] >= 0)
        {
            component->output = (component->output && !component->inputs[i]->output) ||
                                (!component->output && component->inputs[i]->output);
        }
    }
}

void xnorGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = component->inputs[0]->output;
    }
    else
    {
        component->output = false;
    }
    for (int i = 1; i < component->size; i++)
    {
        if (component->inpSrc[i] >= 0)
        {
            component->output = (!component->output && !component->inputs[i]->output) ||
                                (component->output && component->inputs[i]->output);
        }
        else
        {
            component->output = !component->output;
        }
    }
}

void notGate(Component *component)
{
    SetInputs(component);
    if (component->inpSrc[0] >= 0)
    {
        component->output = !component->inputs[0]->output;
    }
    else
    {
        component->output = true;
    }
}

void Tick(Component *component)
{
    if (time == 0)
        component->output = !component->output;
}

void ToggleState(Component *component)
{
    return;
}

void ToggleProbe(Component *component)
{
    SetInputs(component);
    component->output = component->inputs[0]->output;
}
