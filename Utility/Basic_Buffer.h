#pragma once

class Basic_Buffer
{
public:
	Basic_Buffer() {}
	Basic_Buffer(BYTE* b, unsigned long l) {}
	const void * get_buffer() {return NULL;}
	unsigned long value_size() {return 0;}
	int get_spare_capacity_size() {return -1;}
	int set_spare_capacity_size(int x) {return -1;}
	int get_spare_capacity() {return -1;}
	int minimize_capacity() {return -1;}
	int zero_terminate(size_t x) {return -1;}
	int append_spare_capacity_to_data(int x) {return -1;}
	BYTE* release() {return NULL;}
};

class Basic_Output_Buffer
{
public:
	Basic_Output_Buffer(Basic_Buffer* b)
	{

	}
};

class Basic_Output_Deflate
{
public:
	Basic_Output_Deflate(Basic_Output_Buffer* bb, bool is_what){}
	int begin_compressing() {return -1;}
	int write_n(const void* pvData, unsigned long cbData, unsigned long* written) {return -1;}
	int flush() {return -1;}
	int finish_compressing() {return -1;}
};

class Basic_Input_Buffer
{
public:
	Basic_Input_Buffer(Basic_Buffer* b)
	{

	}
};

class Basic_Input_Inflate
{
public:
	Basic_Input_Inflate(Basic_Input_Buffer* bb, bool is_what){}
	int begin_decompression() {return -1;}
	int try_read(int x, int y, unsigned long* z) {return -1;}
	int is_finished() {return -1;}
	int finish_decompression() {return -1;}
};

