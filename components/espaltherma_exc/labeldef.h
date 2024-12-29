#pragma once

#ifdef ARDUINO
#include <pgmspace.h>
#endif
#define LABELDEF

namespace esphome {
namespace espaltherma {

class LabelDef
{
public:
    int convid;
    int offset;
    int registryID;
    int dataSize;
    int dataType;
    const char *label;
    const char* unit_or_type; // unit or type [bool, number, string]
    const char *id;
    // -----------------
    enum EntityType {
      ET_NONE= 0,
      ET_SENSOR,
      ET_TEXTSENSOR,
      ET_BINARYSENSOR
    } entityType;
    EntityBase * pEntity;
    // -----------------
    char *data;
    char asString[30];
    LabelDef(){};
    LabelDef(int registryIDp, int offsetp, int convidp, int dataSizep, int dataTypep, const char *labelp, const char* unitType, const char *idp)
      :
      convid(convidp),
      offset(offsetp),
      registryID(registryIDp),
      dataSize(dataSizep),
      dataType(dataTypep),
      label(labelp),
      unit_or_type(unitType),
      id(idp),
      entityType(ET_NONE),
      pEntity(NULL)
    {};

    EntityType getEntityType() const {
      return entityType;
    }
};

}  // namespace espaltherma
}  // namespace esphome