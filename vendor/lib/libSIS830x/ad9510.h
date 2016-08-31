#ifndef AD9510_H_INCLUDED
#define AD9510_H_INCLUDED

// default setup
SIS830X_STATUS AD9510_SPI_Setup(PSIS830X_DEVICE device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd);

#endif // AD9510_H_INCLUDED
