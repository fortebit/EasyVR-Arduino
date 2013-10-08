/** @file
EasyVR library v1.3
Copyright (C) 2011 RoboTech srl

Written for Arduino and compatible boards for use with EasyVR modules or
EasyVR Shield boards produced by VeeaR <www.veear.eu>

Released under the terms of the MIT license, as found in the accompanying
file COPYING.txt or at this address: <http://www.opensource.org/licenses/MIT>
*/

#include <Stream.h>
#include <stdint.h>

/*****************************************************************************/

#ifndef EASYVR_RX_TIMEOUT
#define EASYVR_RX_TIMEOUT  100  //  default receive timeout (in ms)
#endif

#ifndef EASYVR_WAKE_TIMEOUT
#define EASYVR_WAKE_TIMEOUT  200  // wakeup max delay (in ms)
#endif

#ifndef EASYVR_PLAY_TIMEOUT
#define EASYVR_PLAY_TIMEOUT  5000  // playback max duration (in ms)
#endif

#ifndef EASYVR_TOKEN_TIMEOUT
#define EASYVR_TOKEN_TIMEOUT  1500  // token max duration (in ms)
#endif

/**
  An implementation of the %EasyVR communication protocol.
*/
class EasyVR
{
private:
  Stream* _s;

  uint8_t _value;

  union
  {
    uint8_t v;
    struct
    {
      uint8_t _command : 1;
      uint8_t _builtin : 1;
      uint8_t _error : 1;
      uint8_t _timeout : 1;
      uint8_t _invalid : 1;
      uint8_t _memfull : 1;
      uint8_t _conflict : 1;
      uint8_t _token : 1;
    }
    b;
  }
  _status;
  
  enum
  {
      NO_TIMEOUT = 0, INFINITE = -1,
      DEF_TIMEOUT = EASYVR_RX_TIMEOUT,
      WAKE_TIMEOUT = EASYVR_WAKE_TIMEOUT,
      PLAY_TIMEOUT = EASYVR_PLAY_TIMEOUT,
      TOKEN_TIMEOUT = EASYVR_TOKEN_TIMEOUT,
      RESET_TIMEOUT = 40000,
  };

  void send(uint8_t c);
  void sendCmd(uint8_t c) { _s->flush(); send(c); }
  void sendArg(int8_t c);
  void sendGroup(int8_t c);
  int recv(int16_t timeout = INFINITE);
  bool recvArg(int8_t& c, int16_t timeout = INFINITE);
    
public:
  /** Module identification number (firmware version) */
  enum ModuleId
  {
    VRBOT,    /**< Identifies a VRbot module */
    EASYVR,   /**< Identifies an EasyVR module */
    EASYVR2,  /**< Identifies an EasyVR module version 2 */
  };
  /** Language to use for recognition of built-in words */
  enum Language
  {
    ENGLISH,  /**< Uses the US English word sets */
    ITALIAN,  /**< Uses the Italian word sets */
    JAPANESE, /**< Uses the Japanese word sets */
    GERMAN,   /**< Uses the German word sets */
    SPANISH,  /**< Uses the Spanish word sets */
    FRENCH,   /**< Uses the French word sets */
  };
  /** Special group numbers for recognition of custom commands */
  enum Group
  {
    TRIGGER = 0,    /**< The trigger group (shared with built-in trigger word) */
    PASSWORD = 16,  /**< The password group (uses speaker verification technology) */
  };
  /** Index of built-in word sets */
  enum Wordset
  {
    TRIGGER_SET,    /**< The built-in trigger word set */
    ACTION_SET,     /**< The built-in action word set */
    DIRECTION_SET,  /**< The built-in direction word set */
    NUMBER_SET,     /**< The built-in number word set */
  };
  /** Confidence thresholds for the knob settings,
  used for recognition of built-in words or custom grammars
  (not used for the mixed trigger group) */
  enum Knob
  {
    LOOSER,   /**< Lowest threshold, most results reported */
    LOOSE,    /**< Lower threshold, more results reported */
    TYPICAL,  /**< Typical threshold (deafult) */
    STRICT,   /**< Higher threshold, fewer results reported */
    STRICTER, /**< Highest threshold, fewest results reported */
  };
  /** Strictness values for the level settings,
  used for recognition of custom commands
  (not used for the mixed trigger group) */
  enum Level
  {
    EASY = 1, /**< Lowest value, most results reported */
    NORMAL,   /**< Typical value (default) */
    HARD,     /**< Slightly higher value, fewer results reported */
    HARDER,   /**< Higher value, fewer results reported */
    HARDEST,  /**< Highest value, fewest results reported */
  };
  /** Constants to use for baudrate settings */
  enum Baudrate
  {
    B115200 = 1,  /**< 115200 bps */
    B57600 = 2,   /**< 57600 bps */
    B38400 = 3,   /**< 38400 bps */
    B19200 = 6,   /**< 19200 bps */
    B9600 = 12,   /**< 9600 bps (default) */
  };
  /** Constants for choosing wake-up method in sleep mode */
  enum WakeMode
  {
    WAKE_ON_CHAR = 0,       /**< Wake up on any character received */
    WAKE_ON_WHISTLE = 1,    /**< Wake up on whistle or any character received */
    WAKE_ON_LOUDSOUND = 2,  /**< Wake up on a loud sound or any character received */
    WAKE_ON_2CLAPS = 3,     /**< Wake up on double hands-clap or any character received */
    WAKE_ON_3CLAPS = 6,     /**< Wake up on triple hands-clap or any character received */
  };
  /** Hands-clap sensitivity for wakeup from sleep mode.
  Use in combination with #WAKE_ON_2CLAPS or #WAKE_ON_3CLAPS */
  enum ClapSense
  {
    CLAP_SENSE_LOW = 0,      /**< Lowest threshold */
    CLAP_SENSE_MID = 1,      /**< Typical threshold */
    CLAP_SENSE_HIGH = 2,     /**< Highest threshold */
  };
  /** Pin configuration options for the extra I/O connector */
  enum PinConfig
  {
    OUTPUT_LOW,     /**< Pin is a low output (0V) */
    OUTPUT_HIGH,    /**< Pin is a high output (3V) */
    INPUT_HIZ,      /**< Pin is an high impedance input */
    INPUT_STRONG,   /**< Pin is an input with strong pull-up (~10K) */
    INPUT_WEAK,     /**< Pin is an input with weak pull-up (~200K) */
  };
  /** Available pin numbers on the extra I/O connector */
  enum PinNumber
  {
    IO1 = 1,  /**< Pin IO1 */
    IO2 = 2,  /**< Pin IO2 */
    IO3 = 3,  /**< Pin IO3 */
  };
  /** Some quick volume settings for the sound playback functions
  (any value in the range 0-31 can be used) */
  enum SoundVolume
  {
    VOL_MIN = 0,      /**< Lowest volume (almost mute) */
    VOL_HALF = 7,     /**< Half scale volume (softer) */
    VOL_FULL = 15,    /**< Full scale volume (normal) */
    VOL_DOUBLE = 31,  /**< Double gain volume (louder) */
  };
  /** Special sound index values, always available even when no soundtable is present */
  enum SoundIndex
  {
    BEEP = 0,   /**< Beep sound */
  };
  /** Flags used by custom grammars */
  enum GrammarFlag
  {
    GF_TRIGGER = 0x10,  /**< A bit mask that indicate grammar is a trigger (opposed to commands) */
  };
  /** Noise rejection level for SonicNet token detection (higher value, fewer results) */
  enum RejectionLevel
  {
    REJECTION_MIN,    /**< Lowest noise rejection, highest sensitivity */
    REJECTION_AVG,    /**< Medium noise rejection, medium sensitivity */
    REJECTION_MAX,    /**< Highest noise rejection, lowest sensitivity */
  };
  /** Error codes used by various functions */
  enum ErrorCode
  {
    //-- 0x: Data collection errors (patgen, wordspot, t2si)
    ERR_DATACOL_TOO_LONG        = 0x02, /**< too long (memory overflow) */
    ERR_DATACOL_TOO_NOISY       = 0x03, /**< too noisy */
    ERR_DATACOL_TOO_SOFT        = 0x04, /**< spoke too soft */
    ERR_DATACOL_TOO_LOUD        = 0x05, /**< spoke too loud */
    ERR_DATACOL_TOO_SOON        = 0x06, /**< spoke too soon */
    ERR_DATACOL_TOO_CHOPPY      = 0x07, /**< too many segments/too complex */
    ERR_DATACOL_BAD_WEIGHTS     = 0x08, /**< invalid SI weights */
    ERR_DATACOL_BAD_SETUP       = 0x09, /**< invalid setup */

    //-- 1x: Recognition errors (si, sd, sv, train, t2si)
    ERR_RECOG_FAIL              = 0x11, /**< recognition failed */
    ERR_RECOG_LOW_CONF          = 0x12, /**< recognition result doubtful */
    ERR_RECOG_MID_CONF          = 0x13, /**< recognition result maybe */
    ERR_RECOG_BAD_TEMPLATE      = 0x14, /**< invalid SD/SV template */
    ERR_RECOG_BAD_WEIGHTS       = 0x15, /**< invalid SI weights */
    ERR_RECOG_DURATION          = 0x17, /**< incompatible pattern durations */

    //-- 2x: T2si errors (t2si)
    ERR_T2SI_EXCESS_STATES      = 0x21, /**< state structure is too big */
    ERR_T2SI_BAD_VERSION        = 0x22, /**< RSC code version/Grammar ROM dont match */
    ERR_T2SI_OUT_OF_RAM         = 0x23, /**< reached limit of available RAM */
    ERR_T2SI_UNEXPECTED         = 0x24, /**< an unexpected error occurred */
    ERR_T2SI_OVERFLOW           = 0x25, /**< ran out of time to process */
    ERR_T2SI_PARAMETER          = 0x26, /**< bad macro or grammar parameter */

    ERR_T2SI_NN_TOO_BIG         = 0x29, /**< layer size out of limits */
    ERR_T2SI_NN_BAD_VERSION     = 0x2A, /**< net structure incompatibility */
    ERR_T2SI_NN_NOT_READY       = 0x2B, /**< initialization not complete */
    ERR_T2SI_NN_BAD_LAYERS      = 0x2C, /**< not correct number of layers */

    ERR_T2SI_TRIG_OOV           = 0x2D, /**< trigger recognized Out Of Vocabulary */
    ERR_T2SI_TOO_SHORT          = 0x2F, /**< utterance was too short */

    //-- 4x: Synthesis errors (talk, sxtalk)
    ERR_SYNTH_BAD_VERSION       = 0x4A, /**< bad release number in speech file */
    ERR_SYNTH_ID_NOT_SET        = 0x4B, /**< (obsolete) bad sentence structure */
    ERR_SYNTH_TOO_MANY_TABLES   = 0x4C, /**< (obsolete) too many talk tables */
    ERR_SYNTH_BAD_SEN           = 0x4D, /**< (obsolete) bad sentence number */
    ERR_SYNTH_BAD_MSG           = 0x4E, /**< bad message data or SX technology files missing */

    //-- 8x: Custom errors
    ERR_CUSTOM_NOTA             = 0x80, /**< none of the above (out of grammar) */

    //-- Cx: Internal errors (all)
    ERR_SW_STACK_OVERFLOW       = 0xC0, /**< no room left in software stack */
    ERR_INTERNAL_T2SI_BAD_SETUP = 0xCC, /**< T2SI test mode error */
  };

  /**
    Creates an EasyVR object, using a communication object implementing the 
    #Stream interface (such as #HardwareSerial, or the modified #SoftwareSerial
    and #NewSoftSerial).
    @param s the Stream object to use for communication with the EasyVR module
  */
  EasyVR(Stream& s) : _s(&s) { };
  /**
    Detects an EasyVR module, waking it from sleep mode and checking
    it responds correctly.
    @retval true if a compatible module has been found
  */
  bool detect();
  /**
    Interrupts pending recognition or playback operations.
    @retval true if the request is satisfied and the module is back to ready
  */
  bool stop();
  /**
    Gets the module identification number (firmware version).
    @retval integer is one of the values in #ModuleId
  */
  int8_t getID();
  /**
    Sets the language to use for recognition of built-in words.
    @param lang (0-5) is one of values in #Language
    @retval true if the operation is successful
  */
  bool setLanguage(int8_t lang);
  /**
    Sets the timeout to use for any recognition task.
    @param seconds (0-31) is the maximum time the module keep listening
    for a word or a command
    @retval true if the operation is successful
  */
  bool setTimeout(int8_t seconds);
  /**
    Sets the confidence threshold to use for recognition of built-in words.
    @param knob (0-4) is one of values in #Knob
    @retval true if the operation is successful
  */
  bool setKnob(int8_t knob);
  /**
    Sets the strictness level to use for recognition of custom commands.
    @param level (1-5) is one of values in #Level
    @retval true if the operation is successful
  */
  bool setLevel(int8_t level);
  /**
    Sets the delay before any reply of the module.
    @param millis (0-1000) is the delay duration in milliseconds, rounded to
    10 units in range 10-100 and to 100 units in range 100-1000.
    @retval true if the operation is successful
  */
  bool setDelay(uint16_t millis);
  /**
    Sets the new communication speed. You need to modify the baudrate of the
    underlying Stream object accordingly, after the function returns successfully.
    @param baud is one of values in #Baudrate
    @retval true if the operation is successful
  */
  bool changeBaudrate(int8_t baud);
  /**
    Puts the module in sleep mode.
    @param mode is one of values in #WakeMode, optionally combined with one of
    the values in #ClapSense
    @retval true if the operation is successful
  */
  bool sleep(int8_t mode);
  // command management
  /**
    Adds a new custom command to a group.
    @param group (0-16) is the target group, or one of the values in #Groups
    @param index (0-31) is the index of the command within the selected group
    @retval true if the operation is successful
  */
  bool addCommand(int8_t group, int8_t index);
  /**
    Removes a custom command from a group.
    @param group (0-16) is the target group, or one of the values in #Groups
    @param index (0-31) is the index of the command within the selected group
    @retval true if the operation is successful
  */
  bool removeCommand(int8_t group, int8_t index);
  /**
    Sets the name of a custom command.
    @param group (0-16) is the target group, or one of the values in #Groups
    @param index (0-31) is the index of the command within the selected group
    @retval true if the operation is successful
  */
  bool setCommandLabel(int8_t group, int8_t index, const char* name);
  /**
    Erases the training data of a custom command.
    @param group (0-16) is the target group, or one of the values in #Groups
    @param index (0-31) is the index of the command within the selected group
    @param name is a string containing the label to be assigned to the
    specified command
    @retval true if the operation is successful
  */
  bool eraseCommand(int8_t group, int8_t index);
  // command discovery
  /**
    Gets a bit mask of groups that contain at least one command.
    @param mask is a variable to hold the group mask when the function returns
    @retval true if the operation is successful
  */
  bool getGroupMask(uint32_t& mask);
  /**
    Gets the number of commands in the specified group.
    @param group (0-16) is the target group, or one of the values in #Groups
    @retval integer is the count of commands (negative in case of errors)
  */
  int8_t getCommandCount(int8_t group);
  /**
    Retrieves the name and training data of a custom command.
    @param group (0-16) is the target group, or one of the values in #Groups
    @param index (0-31) is the index of the command within the selected group
    @param name points to an array of at least 32 characters that holds the
    command label when the function returns
    @param training is a variable that holds the training count when the
    function returns. Additional information about training is available
    through the functions #isConflict() and #getWord() or #getCommand()
    @retval true if the operation is successful
  */
  bool dumpCommand(int8_t group, int8_t index, char* name, uint8_t& training);
  // custom grammars
  /**
    Gets the total number of grammars available, including built-in and custom.
    @retval integer is the count of grammars (negative in case of errors)
  */
  int8_t getGrammarsCount(void);
  /**
    Retrieves the contents of a built-in or a custom grammar.
    Command labels contained in the grammar can be obtained by calling #getNextWordLabel()
    @param grammar (0-31) is the target grammar, or one of the values in #Wordset
    @param flags is a variable that holds some grammar flags when the function
    returns. See #GrammarFlag
    @param count is a variable that holds the number of words in the grammar when
    the function returns.
    @retval true if the operation is successful
  */
  bool dumpGrammar(int8_t grammar, uint8_t& flags, uint8_t& count);
  /**
    Retrieves the name of a command contained in a custom grammar.
    It must be called after #dumpGrammar()
    @param name points to an array of at least 32 characters that holds the
    command label when the function returns
    @retval true if the operation is successful
  */
  bool getNextWordLabel(char* name);
  // recognition/training
  /**
    Starts training of a custom command. Results are available after
    #hasFinished() returns true.
    @param group (0-16) is the target group, or one of the values in #Groups
    @param index (0-31) is the index of the command within the selected group
    @note The module is busy until training completes and it cannot
    accept other commands. You can interrupt training with #stop().
  */
  void trainCommand(int8_t group, int8_t index);
  /**
    Starts recognition of a custom command. Results are available after
    #hasFinished() returns true.
    @param group (0-16) is the target group, or one of the values in #Groups
    @note The module is busy until recognition completes and it cannot
    accept other commands. You can interrupt recognition with #stop().
  */
  void recognizeCommand(int8_t group);
  /**
    Starts recognition of a built-in word. Results are available after
    #hasFinished() returns true.
    @param wordset (0-3) is the target word set, or one of the values in
    #Wordset, (4-31) is the target custom grammar, if present
    @note The module is busy until recognition completes and it cannot
    accept other commands. You can interrupt recognition with #stop().
  */
  void recognizeWord(int8_t wordset);
  /**
    Polls the status of on-going recognition, training or asynchronous
    playback tasks.
    @retval true if the operation has completed
  */
  bool hasFinished();
  // analyse result
  /**
    Gets the recognised command index if any.
    @retval (0-31) is the command index if recognition is successful, (-1) if no
    command has been recognized or an error occurred
  */
  int8_t getCommand() { return _status.b._command ? _value : -1; }
  /**
    Gets the recognised word index if any, from built-in sets or custom grammars.
    @retval (0-31) is the command index if recognition is successful, (-1) if no
    built-in word has been recognized or an error occurred
  */
  int8_t getWord() { return _status.b._builtin ? _value : -1; }
  /**
    Gets the index of the received SonicNet token if any.
    @retval integer is the index of the received SonicNet token (0-255 for 8-bit
    tokens or 0-15 for 4-bit tokens) if detection was successful, (-1) if no
    token has been received or an error occurred
  */
  int16_t getToken() { return _status.b._token ? _value : -1; }
  /**
    Gets the last error code if any.
    @retval (0-255) is the error code, (-1) if no error occurred
  */
  int16_t getError() { return _status.b._error ? _value : -1; }
  /**
    Retrieves the timeout indicator.
    @retval true if a timeout occurred
  */
  bool isTimeout() { return _status.b._timeout; }
  /**
    Retrieves the conflict indicator.
    @retval true is a conflict occurred during training. To know what
    caused the conflict, use #getCommand() and #getWord()
    (only valid for triggers)
  */
  bool isConflict() { return _status.b._conflict; }
  /**
    Retrieves the memory full indicator (only valid after #addCommand()
    returned false).
    @retval true if a command could not be added because of memory size
    constaints (up to 32 custom commands can be created)
  */
  bool isMemoryFull() { return _status.b._memfull; }
  // pin I/O functions
  /**
    Configures an I/O pin as an output and sets its value
    @param pin (1-3) is one of values in #PinNumber
    @param pin (0-1) is one of the output values in #PinConfig,
    or Arduino style HIGH and LOW macros
    @retval true if the operation is successful
  */
  bool setPinOutput(int8_t pin, int8_t value);
  /**
    Configures an I/O pin as an input with optional pull-up and
    return its value
    @param pin (1-3) is one of values in #PinNumber
    @param pin (2-4) is one of the input values in #PinConfig
    @retval integer is the logical value of the pin
  */
  int8_t getPinInput(int8_t pin, int8_t config);
  // sound table functions
  /**
    Starts listening for a SonicNet token. Manually check for
    completion with #hasFinished().
    @param bits (4 or 8) specifies the length of received tokens
    @param rejection (0-2) specifies the noise rejection level,
    it can be one of the values in #RejectionLevel
    @param timeout (1-28090) is the maximum time in milliseconds to keep
    listening for a valid token or (0) to listen without time limits.
    @note The module is busy until token detection completes and it cannot
    accept other commands. You can interrupt listening with #stop().
  */
  void detectToken(int8_t bits, int8_t rejection, uint16_t timeout);
  /**
    Starts immediate playback of a SonicNet token. Manually check for
    completion with #hasFinished().
    @param bits (4 or 8) specifies the length of trasmitted token
    @param token is the index of the SonicNet token to play (0-255 for 8-bit
    tokens or 0-15 for 4-bit tokens)
    @note The module is busy until playback completes and it cannot
    accept other commands. You can interrupt playback with #stop().
  */
  void sendTokenAsync(int8_t bits, uint8_t token);
  /**
    Plays a SonicNet token and waits for completion.
    @param bits (4 or 8) specifies the length of trasmitted token
    @param token is the index of the SonicNet token to play (0-255 for 8-bit
    tokens or 0-15 for 4-bit tokens)
    @retval true if the operation is successful
  */
  bool sendToken(int8_t bits, uint8_t token);
  /**
    Schedules playback of a SonicNet token after the next sound starts playing.
    @param bits (4 or 8) specifies the length of trasmitted token
    @param token is the index of the SonicNet token to play (0-255 for 8-bit
    tokens or 0-15 for 4-bit tokens)
    @param delay (1-28090) is the time in milliseconds at which to send the token,
    since the beginning of the next sound playback
    @retval true if the operation is successful
    @note The scheduled token remains valid for one operation only, so you have
    to call #playSound() or #playSoundAsync() immediately after this function.
  */
  bool embedToken(int8_t bits, uint8_t token, uint16_t delay);
  /**
    Starts playback of a sound from the sound table. Manually check for
    completion with #hasFinished().
    @param index is the index of the target sound in the sound table
    @param volume (0-31) may be one of the values in #SoundVolume
    @note The module is busy until playback completes and it cannot
    accept other commands. You can interrupt playback with #stop().
  */
  void playSoundAsync(int16_t index, int8_t volume);
  /**
    Plays a sound from the sound table and waits for completion
    @param index is the index of the target sound in the sound table
    @param volume (0-31) may be one of the values in #SoundVolume
    @retval true if the operation is successful
    @note To alter the maximum time for the wait, define the 
    EASYVR_PLAY_TIMEOUT macro before including the EasyVR library.
  */
  bool playSound(int16_t index, int8_t volume);
  /**
    Retrieves the name of the sound table and the number of sounds it contains
    @param name points to an array of at least 32 characters that holds the
    sound table label when the function returns
    @param count is a variable that holds the number of sounds when the
    function returns
    @retval true if the operation is successful
  */
  bool dumpSoundTable(char* name, int16_t& count);
  /**
    Plays a phone tone and waits for completion
    @param tone is the index of the tone (0-9 for digits, 10 for '*' key, 11
    for '#' key and 12-15 for extra keys 'A' to 'D', -1 for the dial tone)
    @param duration (1-32) is the tone duration in 40 milliseconds units, or
    in seconds for the dial tone
    @retval true if the operation is successful
  */
  bool playPhoneTone(int8_t tone, uint8_t duration);
  /**
    Empties internal memory for custom commands and groups.
    @retval true if the operation is successful
    @note It will take about 35 seconds for the whole process to complete
    and it cannot be interrupted. During this time the module cannot
    accept any other command. The sound table and custom grammars data is not affected.
  */
  bool resetAll();
};

#include "EasyVRBridge.h"
