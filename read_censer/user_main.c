// 明度センサー読み取り

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <tk/device.h>

#define SENSOR_MAX  4

const UB sensor_addr[SENSOR_MAX] = {0x38, 0x39, 0x3c, 0x3d};

typedef struct {
  UH red;
  UH green;
  UH blue;
  UH clear;
} T_COLOR_DATA;

T_COLOR_DATA  cdata[SENSOR_MAX];

ER read_sensor(ID dd, T_COLOR_DATA cdata[]){
  T_I2C_EXEC  exec;
  UB          snd_data;
  UB          rcv_data;
  UH          sens_data[4];
  INT i;
  SZ  asz;
  ER  err;

  for (i = 0; i < SENSOR_MAX; i++){
    while(1){
      exec.sadr = sensor_addr[i];
      snd_data   = 0x42;
      exec.snd_size = 1;
      exec.snd_data = &snd_data;
      exec.rcv_size = 1;
      exec.rcv_data = &rcv_data;
      
      err = tk_swri_dev(dd, TDN_I2C_EXEC, &exec, sizeof(T_I2C_EXEC), &asz);

      if (rcv_data & (1 << 7)){
        exec.sadr = sensor_addr[i];
        snd_data   = 0x50;
        exec.snd_size = 1;
        exec.snd_data = &snd_data;
        exec.rcv_size = sizeof(sens_data);
        exec.rcv_data = (UB*)(sens_data);
     
        err = tk_swri_dev(dd, TDN_I2C_EXEC, &exec, sizeof(T_I2C_EXEC), &asz);
        cdata[i].red = sens_data[0];
        cdata[i].green = sens_data[1];
        cdata[i].blue = sens_data[2];
        cdata[i].clear = sens_data[3];
      
        break;
      }
    }
  }

  return err;
}

EXPORT	INT	usermain( void ) 
{
  UB  data[2];
  SZ  asz;
  INT i;
  ER  err;
  ID  dd;

  dd = tk_opn_dev("iicb", TD_UPDATE);

  for (i = 0; i < SENSOR_MAX; i++){
    data[0] = 0x40; //SYSTEM_CONTROLレジスタ
    data[1] = 0x80; //最上位ビットを1にしてリセット
    err = tk_swri_dev(dd, sensor_addr[i], &data, 2, &asz);

    data[0] = 0x40; //SYSTEM_CONTROLレジスタ
    data[1] = 0x00; //最上位ビットを0にして動作開始
    err = tk_swri_dev(dd, sensor_addr[i], &data, 2, &asz);

    data[0] = 0x42; //MODE_CONTROL2レジスタ
    data[1] = 0x10; //bit4を1にセットしてセンサを有効可
    err = tk_swri_dev(dd, sensor_addr[i], &data, 2, &asz);
  }

  while(1){
    err = read_sensor(dd, cdata);
    tm_printf("right1:%d right2:%d   left2:%d left1:%d\n", cdata[0].clear, cdata[1].clear, cdata[2].clear, cdata[3].clear);
    tk_dly_tsk(1000);
  }
}
