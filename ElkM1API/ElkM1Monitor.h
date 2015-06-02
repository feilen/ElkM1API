/*
	ElkM1Monitor.h: Provides a service that connects to the M1 and handles messages coming in asynchronously, with
	  a cache that notifies waiting clients when data has been written to.
	@author Zach Jaggi
*/

#pragma once
#include "ElkM1API.h"

namespace Elk {
	/*
	Class which ensures packets get loaded one at a time, and can be filtered/rejected based on what they belong to.
	Keeps a cache of the internal state of the M1, and consults it to fulfill requests. This cache is updated by the
	information coming in from the M1.
	Private versions of the functions (left abstract) and a couple protocol-specific functions (also left abstract)
	are implemented in deriving classes (which implement protocols) and deal with actually sending information.
	*/
	class M1Monitor : protected M1API{
	private:
		std::vector<char> buffer;
		bool sigStop = false;
		std::thread executionThread;
		void _start();
	protected:
		// Stores a single object and a timestamp that updates when it is.
		// Allows threads to block until data has been updated, with timeout.
		template <class T> class cacheObject {
			T object;
			std::time_t updateTime;
			std::mutex mutex;
			std::condition_variable newData;
		public:
			cacheObject(T object) {
				this->object = object;
				this->updateTime = std::time(nullptr);
			}
			cacheObject() {
				this->updateTime = time_t(0);
			}
			T get() { 
				std::unique_lock<std::mutex> lock(mutex);
				return object; 
			}
			T awaitNew(int milliseconds = 0)
			{
				std::unique_lock<std::mutex> lock(mutex);
				if (milliseconds <= 0) {
					newData.wait(lock);
					return object;
				}
				else {
					if (newData.wait_for(lock,
						std::chrono::milliseconds(milliseconds)) == std::cv_status::no_timeout)
					{
						return object;
					}
					else {
						throw std::runtime_error("Timed out waiting for response.");
					}
				}
			}
			// Age, in seconds
			int age() {
				std::unique_lock<std::mutex> lock(mutex);
				return std::time(nullptr) - updateTime;
			}
			void invalidate() {
				std::unique_lock<std::mutex> lock(mutex);
				updateTime = std::time(0);
			}
			bool isInitialized() {
				std::unique_lock<std::mutex> lock(mutex);
				return updateTime != 0;
			}
			void set(T object) {
				{
					std::unique_lock<std::mutex> lock(mutex);
					this->object = object;
					this->updateTime = std::time(nullptr);
				}
				newData.notify_all();
			}
		};
		// Stores the last recieved state of various features of the M1, and a timestamp/mutex.
		// Objects which can independently be accessed need a const array, others should be a single object.
		// TODO: Keep a log of button press events <KC>
		class M1Cache {
		public: 
			cacheObject<std::array<ArmStatus, 8>> armStatus;
			cacheObject<std::array<ZoneDefinition,208>> zoneAlarms;
			cacheObject<AudioData> audioData[18];
			//std::array<bool,208> getControlOutputs() = 0; TODO: Replace with better packed function.
			cacheObject<std::array<bool,208>> controlOutputs;
			cacheObject<uint16_t> customValues[20];
			//UserCodeSuccess requestChangeUserCode(int user, std::string authCode, std::string newUserCode, uint8_t areaMask) = 0;
			cacheObject<int> userCodeChanged;
			cacheObject<uint16_t> counterValues[64]; 
			//int getLightingStatus(int device) = 0;
			cacheObject<int> lightingStatus[256];
			cacheObject<std::array<int, 16>> keypadAreas;
			cacheObject<KeypadFkeyStatus> keypadStatuses[16];
			//std::array<ChimeMode, 8> pressFunctionKey(int keypad, FKEY key) = 0;
			// TODO: This also tracks what button was last pressed, so implement that
			cacheObject<std::array<ChimeMode,8>> chimeModes;
			cacheObject<LogEntry> logData[511];
			cacheObject<std::array<int,64>> plcStatus[4];
			cacheObject<RTCData> rtcData;
			cacheObject<std::string> ZoneNames[208];
			cacheObject<std::string> AreaNames[8];
			cacheObject<std::string> UserNames[199];
			cacheObject<std::string> KeypadNames[16];
			cacheObject<std::string> OutputNames[64]; // >64 returns ""
			cacheObject<std::string> TaskNames[32];
			cacheObject<std::string> TelephoneNames[8]; 
			cacheObject<std::string> LightNames[256];
			cacheObject<std::string> AlarmDurationNames[12];
			cacheObject<std::string> CustomSettingNames[20];
			cacheObject<std::string> CounterNames[64];
			cacheObject<std::string> ThermostatNames[16];
			cacheObject<std::string> FKEY1s[16];
			cacheObject<std::string> FKEY2s[16];
			cacheObject<std::string> FKEY3s[16];
			cacheObject<std::string> FKEY4s[16];
			cacheObject<std::string> FKEY5s[16]; 
			cacheObject<std::string> FKEY6s[16];
			cacheObject<std::string> AudioZoneNames[18]; // May be XEP exclusive.
			cacheObject<std::string> AudioSourceNames[12]; // May be XEP exclusive.
			cacheObject<SystemTroubleStatus> systemTroubleStatus;
			// Three different commands update this. Why? I have no idea.
			cacheObject<int> keypadTemperatures[16];
			cacheObject<int> zoneTemperatures[16];
			cacheObject<int> thermostatTemperatures[16];
			cacheObject<ThermostatData> thermostatData[16];
			cacheObject<std::array<int, 3>> M1VersionNumber;
			cacheObject<bool> zonesBypassed[208];
			cacheObject<bool> areaBypassed;
			cacheObject<std::array<ZoneDefinition, 208>> zoneDefinitions;
			cacheObject<std::array<int,208>> zonePartitions;
			//std::array<zoneState, 208> getZoneStates() = 0;
			cacheObject<std::array<ZoneState,208>> zoneStatus;
			cacheObject<float> zoneVoltage[208];
			//Just triggered when "OK\r\n" comes in.
			cacheObject<bool> okMessage;
			//Create objects as needed for this, and then wait on them. If they exist, return from it.
			// std::unordered_map<std::string, cacheObject<UserCodeAccess>> userCodeAcesses;
			// TODO: Implement concurrent unordered_map implementation similar to cacheObject

			// TODO: Implement
			void invalidate() {}
		} m1cache;
		// Handle incoming messages, use them to update cache.
		virtual void handleMessage(std::vector<char> message) = 0;
		// Cut a message from our buffer and delete it. If no messages, return empty vector.
		virtual std::vector<char> cutMessage(std::vector<char>& buffer) = 0;
		M1Connection* connection;
	public:
		// Construct a new Monitor instance. To be viable, the ElkM1Connection must be established before beginning with run().
		ELKM1API M1Monitor(M1Connection* conn);
		ELKM1API ~M1Monitor();
		// Spawn a thread and begin monitoring.
		ELKM1API void run();
		// Instruct the monitoring thread to clean up and exit
		ELKM1API void stop();
	};
}