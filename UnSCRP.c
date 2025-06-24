#define _CRT_SECURE_NO_WARNINGS
#define OUTPUT_GROUPS_DIR "output\\groups\\"
#define OUTPUT_TIMERS_DIR "output\\timers\\"
#define MAX_FILENAME 260
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int reverse_endian_32(unsigned int number) {
	return ((number & 0xFF) << 24) | ((number & 0xFF00) << 8) | ((number & 0xFF0000) >> 8) | (number >> 24);
}

int read_flipped_int(FILE* file) {
	int read;
	fread(&read, sizeof(int), 1, file);
	read = reverse_endian_32(read);
	return read;
}

void write_flipped_int(unsigned int n, FILE* file) {
	n = reverse_endian_32(n);
	fwrite(&n, sizeof(int), 1, file);
}

void write_flipped_short(unsigned short n, FILE* file) {
	char buf1[2] = { 0 };
	memcpy(buf1, &n, sizeof(short));
	char buf2[2] = { buf1[1], buf1[0] };
	fwrite(&buf2, sizeof(short), 1, file);
}

unsigned int bkdr(char* str) {
	unsigned int out = 0;
	for (int i = 0; i < strlen(str); i++) {
		out = out * 131 + str[i];
		while (out >= pow(2, 32)) {
			out -= pow(2, 32);
		}
	}
	return out;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		fprintf(stdout, "Usage: UnSCRP input_scrp output_name\n");
		return 1;
	}

	FILE* input = fopen(argv[1], "rb");
	if (input == NULL) {
		fprintf(stdout, "Could not open file %s for reading\n", argv[1]);
		return 1;
	}

	fseek(input, 0x4, SEEK_SET);
	if (fgetc(input) != 0x2A) {
		fprintf(stdout, "Input asset is not a SCRP\n");
		return 1;
	}

	unsigned char input_link_count = fgetc(input);
	unsigned char input_base_flags[2] = { fgetc(input), fgetc(input) };

	fseek(input, 0xC, SEEK_SET);
	int input_event_count = read_flipped_int(input);

	char out_path[MAX_FILENAME] = { 0 };
	strcpy(out_path, OUTPUT_GROUPS_DIR);
	char group_name[32] = { 0 };
	int added_char_count = strlen("_GROUP") + 1;
	if (strlen(argv[2]) >= 32 - added_char_count) {
		strncpy(group_name, argv[2], 32 - added_char_count);
	}
	else {
		strncpy(group_name, argv[2], strlen(argv[2]));
	}
	strcat(group_name, "_GROUP");
	strcat(out_path, group_name);

	FILE* out_group = fopen(out_path, "wb");
	if (out_group == NULL) {
		fprintf(stdout, "Could not open file %s for writing\n", out_path);
		return 1;
	}

	write_flipped_int(bkdr(group_name), out_group);
	fputc(0x11, out_group); // base type
	fputc(input_link_count, out_group); // link count
	fputc(input_base_flags[0], out_group); // base flags
	fputc(input_base_flags[1], out_group);
	write_flipped_short(input_event_count, out_group);
	fputc(0x0, out_group);
	fputc(0x0, out_group);

	fseek(input, 0x14, SEEK_SET);
	for (int i = 0; i < input_event_count; i++) {
		char timer_name[32] = { 0 };
		char n[10] = { 0 };

		snprintf(n, 10, "%d", i);
		int added_char_count = strlen("_TIMER_") + strlen(n) + (i < 10 ? 1 : 0) + 1;
		if (strlen(argv[2]) >= 32 - added_char_count) {
			strncpy(timer_name, argv[2], 32 - added_char_count);
		}
		else {
			strncpy(timer_name, argv[2], strlen(argv[2]));
		}
		strcat(timer_name, "_TIMER_");
		if (i < 10) {
			strcat(timer_name, "0");
		}
		strcat(timer_name, n);

		char out_path[MAX_FILENAME] = { 0 };
		strcpy(out_path, OUTPUT_TIMERS_DIR);
		strcat(out_path, timer_name);
		
		FILE* out_timer = fopen(out_path, "wb");
		if (out_timer == NULL) {
			fprintf(stdout, "Could not open file %s for writing\n", out_path);
			return 1;
		}

		unsigned int hash = bkdr(timer_name);
		write_flipped_int(hash, out_timer);
		fputc(0x0E, out_timer); // base type
		fputc(0x01, out_timer); // link count
		fputc(0x00, out_timer); // base flags
		fputc(0x1D, out_timer);

		float time;
		fread(&time, sizeof(float), 1, input);
		int ass;
		fread(&ass, sizeof(int), 1, input);
		fseek(input, 0x2, SEEK_CUR);
		short dst;
		fread(&dst, sizeof(short), 1, input);
		int args[6] = { 0 };
		fread(&args, sizeof(int), 5, input);

		fwrite(&time, sizeof(float), 1, out_timer);
		time = 0.0f;
		fwrite(&time, sizeof(float), 1, out_timer);
		fputc(0x00, out_timer);
		fputc(0x14, out_timer);
		fwrite(&dst, sizeof(short), 1, out_timer);
		fwrite(&ass, sizeof(int), 1, out_timer);
		fwrite(&args, sizeof(int), 6, out_timer);

		fclose(out_timer);

		write_flipped_int(hash, out_group);
	}

	char* buf = calloc(input_link_count, 0x20);
	if (buf == NULL) {
		fprintf(stdout, "Not enough memory\n");
		return 1;
	}

	fread(buf, 0x20, input_link_count, input);
	fwrite(buf, 0x20, input_link_count, out_group);

	fprintf(stdout, "Successfully created group and %d timers in \"output\" folder.\n", input_event_count);

	free(buf);
	fclose(out_group);
	fclose(input);
	return 0;
}