# /*
#  *  YDTriggerר�ú�
#  *  
#  *  By actboy168
#  *
#  */
#
#ifndef INCLUDE_YDTRIGGER_H
#define INCLUDE_YDTRIGGER_H
#
#  ifndef DISABLE_SAVE_LOAD_SYSTEM
#      include <YDTrigger/ImportSaveLoadSystem.h>
#  endif
#
#  ifdef  USE_BJ_OPTIMIZATION 
#
#    include <YDTrigger/BJOptimization/Math.h>
#    include <YDTrigger/BJOptimization/General.h>
#    include <YDTrigger/BJOptimization/Camera.h>
#    include <YDTrigger/BJOptimization/Trigger.h>
#    include <YDTrigger/BJOptimization/Unit.h>
#    include <YDTrigger/BJOptimization/Animation.h>
#    include <YDTrigger/BJOptimization/Destructable.h>
#    include <YDTrigger/BJOptimization/Environment.h>
#    include <YDTrigger/BJOptimization/Group.h>
#    include <YDTrigger/BJOptimization/Hero.h>
#    include <YDTrigger/BJOptimization/Quest.h>
#    include <YDTrigger/BJOptimization/Sound.h>
#
#    ifdef USE_BJ_OPTIMIZATION_PRO
#      include <YDTrigger/BJOptimization/Pro_WidgetLift.h>
#    endif
#  endif
#
#  define YDTriggerEnumUintsInRange(g, x, y, r) GroupEnumUnitsInRange(g, x, y, r, null)
#
#  ifdef USE_BJ_ANTI_LEAK
#    include "AntiBJLeak/MainMacro.h"
#  endif
#
#  define YDWESaveTriggerName(t,s) DoNothing()
#
#endif
