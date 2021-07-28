#include "component.h"

extern Component ComponentList[256];
extern int time;

void GetWidthHeight(int *w, int *h, Type type, int size) {
  if (type == state || type == clock || type == probe) {
    *w = 1;
    *h = 1;
  } else if (type == g_not) {
    *w = 3;
    *h = 1;
  } else {
    *w = 4;
    *h = size;
  }
}

void SetIOPos(Component *component, int inpNum) {
  for (int i = inpNum; i < MAX_INPUT_NUM; i++) {
    component->inpPos[i].y = -1;
    component->inpPos[i].x = -1;
  }
  for (int i = 0; i < inpNum; i++) {
    component->inpPos[i].y = component->start.y + i;
    component->inpPos[i].x = component->start.x;
  }
  component->outPos.x = component->start.x + component->width - 1;
  component->outPos.y = component->start.y + component->size / 2;
}

void ClearInputs(Component *component) {
  for (int i = 0; i < MAX_INPUT_NUM; i++) {
    component->inpSrc[i] = -1;
    component->inputs[i] = false;
  }
}

void SetInputs(Component *component) {
  for (int i = 0; i < component->size; i++) {
    if (component->inpSrc[i] == -1)
      continue;
    Component sender = ComponentList[component->inpSrc[i]];
    component->inputs[i] = sender.output;
  }
}

void andGate(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
  for (int i = 1; i < component->size; i++) {
    component->output = component->output && component->inputs[i];
  }
}

void orGate(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
  for (int i = 1; i < component->size; i++) {
    component->output = component->output || component->inputs[i];
  }
}

void nandGate(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
  for (int i = 1; i < component->size; i++) {
    component->output = !(component->output && component->inputs[i]);
  }
}

void norGate(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
  for (int i = 1; i < component->size; i++) {
    component->output = !(component->output || component->inputs[i]);
  }
}

void xorGate(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
  for (int i = 1; i < component->size; i++) {
    component->output = (component->output && !component->inputs[i]) ||
                        (!component->output && component->inputs[i]);
  }
}

void xnorGate(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
  for (int i = 1; i < component->size; i++) {
    component->output = (!component->output && !component->inputs[i]) ||
                        (component->output && component->inputs[i]);
  }
}

void notGate(Component *component) {
  SetInputs(component);
  component->output = !component->inputs[0];
}

void Tick(Component *component) {
  if (time % 200 == 0)
    component->output = !component->output;
}

void ToggleState(Component *component) {
  component->output = !component->output;
}

void ToggleProbe(Component *component) {
  SetInputs(component);
  component->output = component->inputs[0];
}

Component MakeSingleInputCompo(Type type, Pair pos){
  Component component;
  component.size = 1;
  component.width = 1;
  component.start.x = pos.x;
  component.start.y = pos.y;
  component.type = type;
  ClearInputs(&component);
  switch(type){
    case(g_not):
      component.width = 3;
      component.operate = notGate;
      SetIOPos(&component, 1);
      break;
    case(state):
      component.operate = ToggleState;
      SetIOPos(&component, 0);
      break;
    case(clock):
      component.operate = Tick;
      SetIOPos(&component, 0);
      break;
    default:
      component.operate = ToggleProbe;
      SetIOPos(&component, 1);
      component.outPos.x = -1;
      component.outPos.y = -1;
      break;
  }
  return component;
}

Component MultiInputComponent(Type type, int inpNum, Pair pos) {
  Component component;
  component.start.x = pos.x;
  component.start.y = pos.y;
  component.size = inpNum;
  component.width = 4;
  component.type = type;
  ClearInputs(&component);
  SetIOPos(&component, inpNum);
  switch (type) {
  case (g_and):
    component.operate = andGate;
    break;
  case (g_or):
    component.operate = orGate;
    break;
  case (g_nand):
    component.operate = nandGate;
    break;
  case (g_nor):
    component.operate = norGate;
    break;
  case (g_xor):
    component.operate = xorGate;
    break;
  default:
    component.operate = xnorGate;
    break;
  }
  return component;
}

Component GetComponent(Type type, char inpNum, Pair pos) {
    if (type == state || type == clock || type == g_not || type == probe)
        return MakeSingleInputCompo(type, pos);
    else
        return MultiInputComponent(type, inpNum, pos);
}
