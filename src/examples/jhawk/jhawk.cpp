
#ifndef MODULE_NAME
#define MODULE_NAME "Jhawk"
#endif

#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/tasks.h>
#include <px4_platform_common/posix.h>
#include <px4_platform_common/defines.h>

#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>

#include <drivers/drv_board_led.h>

#include <systemlib/err.h>

#include <board_config.h>

#include "jhawk.h"

Queue _queue;
extern "C" __EXPORT void send_signal(float data1, float data2,  float data3, float scale, int tick);

Jhawk::Jhawk():
	ModuleParams(nullptr),
	ScheduledWorkItem(MODULE_NAME, px4::wq_configurations::hp_default)
{
}


Jhawk::~Jhawk()
{
}

void Jhawk::Run()		//2ms
{
	if(_sensor_gyro_sub.updated()){
		sensor_gyro_s gyro;
		if (_sensor_gyro_sub.update(&gyro)) {
			send_signal(gyro.x, gyro.y,  gyro.z, 10.0f, 10.0f);
		}
	static int cnt;

	if((++cnt%30)==0)
		printf("Hello World!! %d\r\n", cnt);

	}

	uint8_t txBuf[60];
	int len = _queue.get_queue(txBuf, 60);
	if(len > 0){
		fwrite(txBuf, len, 1, stdout);
		fflush(stdout);
	}

#if 0
	hrt_abstime now_us = hrt_absolute_time();

	static int cnt;
	static int cnt3;
	if((++cnt%20)!=0)
		return;

	if(++cnt3>25){
		cnt3 = 0;
//		long t1 = hrt_absolute_time();
//		float rr = testfc(0.6f, 4.3f);
		//_print(1, "res:%f, %ld", (double)rr, hrt_absolute_time()-t1);
		fwrite("Hello World!!\r\n", 16, 1, stdout);
		fflush(stdout);
	}

	packet.option = 1;
	packet.length = 28;
	if(_sensor_gyro_sub.updated()){
		sensor_gyro_s gyro;
		if (_sensor_gyro_sub.update(&gyro)) {
			packet.data8[0] = (int8_t)(gyro.x*10);
			packet.data8[1] = (int8_t)(gyro.y*10);
			packet.data8[2] = (int8_t)(gyro.z*10);
			packet.data8[3] = 0;
		}
	}
	if(_sensor_accel_sub.updated()){
		sensor_accel_s accel;
		if (_sensor_accel_sub.update(&accel)) {
			packet.data8[4] = (int8_t)(accel.x*10);
			packet.data8[5] = (int8_t)(accel.y*10);
			packet.data8[6] = (int8_t)(accel.z*10);
			packet.data8[7] = 0;
		}
	}
	if(_optical_flow_sub.updated()){
		optical_flow_s optical_flow;
		if(_optical_flow_sub.update(&optical_flow)){
			packet.data8[12] = (int8_t)(optical_flow.pixel_flow_x_integral*50);
			packet.data8[13] = (int8_t)(optical_flow.pixel_flow_y_integral*50);
		}
	}

	if(_distance_sensor_sub.updated()){
		distance_sensor_s distance_sensor;
		if(_distance_sensor_sub.update(&distance_sensor)){
			packet.data8[14] = (int8_t)(distance_sensor.current_distance*20);
		}
	}

	if (_magnetometer_sub.updated()) {
		vehicle_magnetometer_s magnetometer;
		_magnetometer_sub.update(&magnetometer);
		packet.data8[16] = (int8_t)(magnetometer.magnetometer_ga[0]*100);
		packet.data8[17] = (int8_t)(magnetometer.magnetometer_ga[1]*100);
		packet.data8[18] = (int8_t)(magnetometer.magnetometer_ga[2]*100);
		packet.data8[19] = 0;
	}

	if((now_us-last_time)>20000){
		for(int n=0;n <4 ;n++){
			packet.data8[8+n] = (int8_t)(motor_output[n]*100);
			packet.data8[12+n] = (int8_t)(debug[n]*1);
		}
		Checksum(&packet);
		write(_fd, (const void *)&packet, packet.length);

		last_time = now_us;
	}
#endif
}

bool Jhawk::init()
{
	if (!_sensor_gyro_sub.registerCallback())
		return false;

	ScheduleNow();
	return true;
}

int Jhawk::task_spawn(int argc, char *argv[])
{
	Jhawk *instance = new Jhawk();

	if (instance) {
		_object.store(instance);
		_task_id = task_id_is_work_queue;

		if (instance->init()) {
			return PX4_OK;
		}

	} else {
		PX4_ERR("alloc failed");
	}

	delete instance;
	_object.store(nullptr);
	_task_id = -1;
	return PX4_ERROR;
}

int Jhawk::custom_command(int argc, char *argv[])
{
	return print_usage("unknown command");
}

int Jhawk::print_usage(const char *reason)
{
	printf("JHawk Module\r\n");
	return 0;
}

extern "C" __EXPORT int jhawk_main(int argc, char *argv[])
{
	return Jhawk::main(argc, argv);
}

uint8_t checksum(uint8_t *pkt)
{
	int n, sum=0;

	for(n=6; n<pkt[4]; n++)
		sum += pkt[n];
	return (sum&0xFF);
}


extern "C" __EXPORT void send_signal(float data1, float data2,  float data3, float scale, int tick)
{
	static int tickCnt;
	static uint8_t cmd[40];
	uint32_t *u32 = (uint32_t *)cmd;
	float *f32 = (float *)cmd;
	u32[0] = 0x6214A826;
	if((++tickCnt%tick) != 0)
		return;
	f32[2] = data1*scale;
	f32[3] = data2*scale;
	f32[4] = data3*scale;
	cmd[4] = 20;
	cmd[6] = 3;
	cmd[5] = checksum(cmd);
	_queue.add_queue(cmd, cmd[4]);
}



