
int as7058_init(const struct i2c_dt_spec *dev);

int as7058_write_reg(const struct i2c_dt_spec *dev, uint8_t reg_addr, uint8_t number, uint8_t* values);
int as7058_read_reg(const struct i2c_dt_spec *dev, uint8_t reg_addr, uint8_t number, uint8_t* values);

int as7058_fifo_read_level(const struct i2c_dt_spec *dev, uint16_t *level);
int as7058_read_fifo_raw(const struct i2c_dt_spec *dev, struct as7058_fifo_t *raw);
int as7058_fifo_read_word(const struct i2c_dt_spec *dev, as7058_sample_t *sample);
int as7058_read_fifo(const struct i2c_dt_spec *dev, as7058_sample_t *samples, uint16_t num_samples);

int as7058_check_status(const struct i2c_dt_spec *dev, uint8_t *status_out);
int as7058_print_status(const struct i2c_dt_spec *dev);

int as7058_start(const struct i2c_dt_spec *dev);
int as7058_stop(const struct i2c_dt_spec *dev);
