#ifdef USES_P214
//#######################################################################################################
//#################################### Plugin 500: HC-SR04 ##############################################
//#######################################################################################################

#define PLUGIN_214
#define PLUGIN_ID_214        214
#define PLUGIN_076_DEBUG     true    //activate extra log info in the debug
#define PLUGIN_NAME_214       "Capacity - TankManager [TESTING]"
#define PLUGIN_VALUENAME1_214 "Liters"
//#define PLUGIN_VALUENAME2_214 "m3"

#include <Arduino.h>
#include <map>
#include <NewPingESP8266.h>

// PlugIn specific defines
// operatingMode
#define OPMODE_VALUE        (0)
#define OPMODE_STATE        (1)

// measuringUnit
#define UNIT_CM             (0)
#define UNIT_INCH           (1)

// filterType
#define FILTER_NONE         (0)
#define FILTER_MEDIAN       (1)

// map of sensors
std::map<unsigned int, std::shared_ptr<NewPingESP8266> > P_214_sensordefs;

boolean Plugin_214(byte function, struct EventStruct *event, String& string)
{
  static byte switchstate[TASKS_MAX];
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_214;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;

        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_214);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_214));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        int16_t operatingMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int16_t threshold = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        int16_t max_distance = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        int16_t measuringUnit = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        int16_t filterType = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        int16_t filterSize = Settings.TaskDevicePluginConfig[event->TaskIndex][5];

        int16_t height = Settings.TaskDevicePluginConfig[event->TaskIndex][6];
        int16_t width = Settings.TaskDevicePluginConfig[event->TaskIndex][7];
        int16_t depth = Settings.TaskDevicePluginConfig[event->TaskIndex][8];
        //float calibration = Settings.TaskDevicePluginConfig[event->TaskIndex][9];

        // default filtersize = 5
        if (filterSize == 0) {
          filterSize = 5;
          Settings.TaskDevicePluginConfig[event->TaskIndex][5] = filterSize;
        }

        String strUnit = (measuringUnit == UNIT_CM) ? F("cm") : F("inch");

        String optionsOpMode[2];
        int optionValuesOpMode[2] = { 0, 1 };
        optionsOpMode[0] = F("Value");
        optionsOpMode[1] = F("State");
        addFormSelector(F("Mode"), F("p214_mode"), 2, optionsOpMode, optionValuesOpMode, operatingMode);

        if (operatingMode == OPMODE_STATE)
        {
          addFormNumericBox(F("Threshold"), F("p214_threshold"), threshold);
          addUnit(strUnit);
        }

        addFormNumericBox(F("Max Distance"), F("p214_max_distance"), max_distance, 0, 400);
        addUnit(strUnit);

        String optionsUnit[2];
        int optionValuesUnit[2] = { 0, 1 };
        optionsUnit[0] = F("Metric");
        optionsUnit[1] = F("Imperial");
        addFormSelector(F("Unit"), F("p214_Unit"), 2, optionsUnit, optionValuesUnit, measuringUnit);

        String optionsFilter[2];
        int optionValuesFilter[2] = { 0, 1 };
        optionsFilter[0] = F("None");
        optionsFilter[1] = F("Median");
        addFormSelector(F("Filter"), F("p214_FilterType"), 2, optionsFilter, optionValuesFilter, filterType);

        // enable filtersize option if filter is used,
        if (filterType != FILTER_NONE)
        	addFormNumericBox(F("Filter size"), F("p214_FilterSize"), filterSize, 2, 20);

        addFormNumericBox(F("Height"), F("p214_Height"), height, 0, 200);
        addFormNumericBox(F("Width"), F("p214_Width"), width, 0, 200);
        addFormNumericBox(F("Depth"), F("p214_Depth"), depth, 0, 200);
        //addFormNumericBox(F("Calibration"), F("p214_Calibration"), calibration, 0, 200);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {

        int16_t operatingMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int16_t filterType = Settings.TaskDevicePluginConfig[event->TaskIndex][4];

        Settings.TaskDevicePluginConfig[event->TaskIndex][0] = getFormItemInt(F("p214_mode"));
        if (operatingMode == OPMODE_STATE)
          Settings.TaskDevicePluginConfig[event->TaskIndex][1] = getFormItemInt(F("p214_threshold"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][2] = getFormItemInt(F("p214_max_distance"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("p214_Unit"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][4] = getFormItemInt(F("p214_FilterType"));
        if (filterType != FILTER_NONE)
          Settings.TaskDevicePluginConfig[event->TaskIndex][5] = getFormItemInt(F("p214_FilterSize"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][3] = getFormItemInt(F("p214_Unit"));

        Settings.TaskDevicePluginConfig[event->TaskIndex][6] = getFormItemInt(F("p214_Height"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][7] = getFormItemInt(F("p214_Width"));
        Settings.TaskDevicePluginConfig[event->TaskIndex][8] = getFormItemInt(F("p214_Depth"));
        //Settings.TaskDevicePluginConfig[event->TaskIndex][9] = getFormItemFloat(F("p214_Calibration"));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        int16_t max_distance = Settings.TaskDevicePluginConfig[event->TaskIndex][2];
        int16_t measuringUnit = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        int16_t filterType = Settings.TaskDevicePluginConfig[event->TaskIndex][4];
        int16_t filterSize = Settings.TaskDevicePluginConfig[event->TaskIndex][5];
        /*
        float height = Settings.TaskDevicePluginConfig[event->TaskIndex][6];
        float width = Settings.TaskDevicePluginConfig[event->TaskIndex][7];
        float depth = Settings.TaskDevicePluginConfig[event->TaskIndex][8];
        float calibration = Settings.TaskDevicePluginConfig[event->TaskIndex][9];
        */
        int8_t Plugin_214_TRIG_Pin = Settings.TaskDevicePin1[event->TaskIndex];
        int8_t Plugin_214_IRQ_Pin = Settings.TaskDevicePin2[event->TaskIndex];
        int16_t max_distance_cm = (measuringUnit == UNIT_CM) ? max_distance : (float)max_distance * 2.54f;

        // create sensor instance and add to std::map
        P_214_sensordefs.erase(event->TaskIndex);
        P_214_sensordefs[event->TaskIndex] =
          std::shared_ptr<NewPingESP8266> (new NewPingESP8266(Plugin_214_TRIG_Pin, Plugin_214_IRQ_Pin, max_distance_cm));

        String log = F("TANKMANAGER : TaskNr: ");
        log += event->TaskIndex +1;
        log += F(" TrigPin: ");
        log += Plugin_214_TRIG_Pin;
        log += F(" IRQ_Pin: ");
        log += Plugin_214_IRQ_Pin;
        log += F(" max dist ");
        log += (measuringUnit == UNIT_CM) ? F("[cm]: ") : F("[inch]: ");
        log += max_distance;
        log += F(" max echo: ");
        log += P_214_sensordefs[event->TaskIndex]->getMaxEchoTime();
        log += F(" Filter: ");
        if (filterType == FILTER_NONE)
          log += F("none");
        else
          if (filterType == FILTER_MEDIAN) {
            log += F("Median size: ");
            log += filterSize;
          }
          else
            log += F("invalid!");
        log += F(" nr_tasks: ");
        log += P_214_sensordefs.size();
        addLog(LOG_LEVEL_INFO, log);

        unsigned long tmpmillis = millis();
        unsigned long tmpmicros = micros();
        delay(500);
        long millispassed = timePassedSince(tmpmillis);
        long microspassed = usecPassedSince(tmpmicros);

        log = F("TANKMANAGER : micros() test: ");
        log += millispassed;
        log += F(" msec, ");
        log += microspassed;
        log += F(" usec, ");
        addLog(LOG_LEVEL_INFO, log);

        success = true;
        break;

      }

    case PLUGIN_EXIT:
      {
        P_214_sensordefs.erase(event->TaskIndex);
        break;
      }

    case PLUGIN_READ: // If we select value mode, read and send the value based on global timer
      {

        int16_t operatingMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int16_t measuringUnit = Settings.TaskDevicePluginConfig[event->TaskIndex][3];
        int16_t height = Settings.TaskDevicePluginConfig[event->TaskIndex][6];
        int16_t width = Settings.TaskDevicePluginConfig[event->TaskIndex][7];
        int16_t depth = Settings.TaskDevicePluginConfig[event->TaskIndex][8];
        //float calibration = Settings.TaskDevicePluginConfig[event->TaskIndex][9];

        if (operatingMode == OPMODE_VALUE)
        {
          float surfaceDistance = Plugin_214_read(event->TaskIndex);
          String log = F("TANKMANAGER : TaskNr: ");
          log += event->TaskIndex +1;
          log += F(" Liters: ");

          //UserVar[event->BaseVarIndex] = Plugin_214_QuantityWater(height - calibration - surfaceDistance, width * depth);
          UserVar[event->BaseVarIndex] = Plugin_214_QuantityWater((float)height - surfaceDistance, (float)width * (float)depth);
          log += UserVar[event->BaseVarIndex];
          log += (measuringUnit == UNIT_CM) ? F(" cm ") : F(" inch ");
          if (surfaceDistance == NO_ECHO)
          {
             log += F(" Error: ");
             log += Plugin_013_getErrorStatusString(event->TaskIndex);
          }

          addLog(LOG_LEVEL_INFO,log);
        }
        success = true;
        break;

      }

    case PLUGIN_TEN_PER_SECOND: // If we select state mode, do more frequent checks and send only state changes
      {

        int16_t operatingMode = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int16_t threshold = Settings.TaskDevicePluginConfig[event->TaskIndex][1];
        /*
        float height = Settings.TaskDevicePluginConfig[event->TaskIndex][6];
        float width = Settings.TaskDevicePluginConfig[event->TaskIndex][7];
        float depth = Settings.TaskDevicePluginConfig[event->TaskIndex][8];
        float calibration = Settings.TaskDevicePluginConfig[event->TaskIndex][9];
        */

        if (operatingMode == OPMODE_STATE)
        {
          byte state = 0;
          float surfaceDistance = Plugin_013_read(event->TaskIndex);
          if (surfaceDistance != NO_ECHO)
          {
            if (surfaceDistance < threshold)
              state = 1;
            if (state != switchstate[event->TaskIndex])
            {
              String log = F("TANKMANAGER : TaskNr: ");
              log += event->TaskIndex +1;
              log += F(" state: ");
              log += state;
              addLog(LOG_LEVEL_INFO,log);
              switchstate[event->TaskIndex] = state;
              UserVar[event->BaseVarIndex] = state;
              event->sensorType = SENSOR_TYPE_SWITCH;
              sendData(event);
            }
          }
          else {
            String log = F("TANKMANAGER : TaskNr: ");
            log += event->TaskIndex +1;
            log += F(" Error: ");
            log += Plugin_214_getErrorStatusString(event->TaskIndex);
            addLog(LOG_LEVEL_INFO,log);
          }
        }
        success = true;
        break;
      }
  }
  return success;
}


/*********************************************************************/
float Plugin_214_read(unsigned int taskIndex)
/*********************************************************************/
{
  if (P_214_sensordefs.count(taskIndex) == 0)
    return 0;

  int16_t max_distance = Settings.TaskDevicePluginConfig[taskIndex][2];
  int16_t measuringUnit = Settings.TaskDevicePluginConfig[taskIndex][3];
  int16_t filterType = Settings.TaskDevicePluginConfig[taskIndex][4];
  int16_t filterSize = Settings.TaskDevicePluginConfig[taskIndex][5];
  int16_t max_distance_cm = (measuringUnit == UNIT_CM) ? max_distance : (float)max_distance * 2.54f;

  unsigned int echoTime = 0;

  switch  (filterType) {
    case FILTER_NONE:
      echoTime = (P_214_sensordefs[taskIndex])->ping();
      break;
    case FILTER_MEDIAN:
      echoTime = (P_214_sensordefs[taskIndex])->ping_median(filterSize, max_distance_cm);
      break;
    default:
      addLog(LOG_LEVEL_INFO, F("invalid Filter Type setting!"));
  }

  if (measuringUnit == UNIT_CM)
    return NewPingESP8266::convert_cm_F(echoTime);
  else
    return NewPingESP8266::convert_in_F(echoTime);
}

/*********************************************************************/
String Plugin_214_getErrorStatusString(unsigned int taskIndex)
/*********************************************************************/
{
  if (P_214_sensordefs.count(taskIndex) == 0)
    return String(F("invalid taskindex"));

  switch ((P_214_sensordefs[taskIndex])->getErrorState()) {
    case NewPingESP8266::STATUS_SENSOR_READY: {
      return String(F("Sensor ready"));
      break;
    }

    case NewPingESP8266::STATUS_MEASUREMENT_VALID: {
      return String(F("no error, measurement valid"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_TRIGGERED: {
      return String(F("Echo triggered, waiting for Echo end"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_STATE_ERROR: {
      return String(F("Echo pulse error, Echopin not low on trigger"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_START_TIMEOUT_50ms: {
      return String(F("Echo timeout error, no echo start whithin 50 ms"));
      break;
    }

    case NewPingESP8266::STATUS_ECHO_START_TIMEOUT_DISTANCE: {
      return String(F("Echo timeout error, no echo start whithin time for max. distance"));
      break;
    }

    case NewPingESP8266::STATUS_MAX_DISTANCE_EXCEEDED: {
      return String(F("Echo too late, maximum distance exceeded"));
      break;
    }

    default: {
      return String(F("unknown error"));
      break;
    }
  }
}

/*********************************************************************/
float Plugin_214_QuantityWater(float lheight, float lwidth_ldepth)
/*********************************************************************/
{
  if (lheight <= 0) lheight = 0;
  return lheight * lwidth_ldepth / 5000;
}

#endif // USES_p214
