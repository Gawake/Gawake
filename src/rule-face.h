#pragma once

#include <adwaita.h>

#define ALLOW_MANAGING_RULES
# include "database-connection/database-connection.h"
#undef ALLOW_MANAGING_RULES

G_BEGIN_DECLS

typedef enum
{
  RULE_FACE_TYPE_TURN_ON,
  RULE_FACE_TYPE_TURN_OFF,

  RULE_FACE_TYPE_LAST
} RuleFaceType;

#define RULE_TYPE_FACE (rule_face_get_type ())

G_DECLARE_FINAL_TYPE (RuleFace, rule_face, RULE, FACE, AdwBin)

RuleFace *rule_face_new (RuleFaceType type);
void rule_face_open_setup_add_dialog (RuleFace *self);

G_END_DECLS

