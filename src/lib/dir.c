/* =============================================================================
 * Pure64 -- a 64-bit OS/software loader written in Assembly for x86-64 systems
 * Copyright (C) 2008-2017 Return Infinity -- see LICENSE.TXT
 * =============================================================================
 */

#include <pure64/dir.h>
#include <pure64/error.h>
#include <pure64/file.h>
#include <pure64/memory.h>
#include <pure64/path.h>
#include <pure64/stream.h>
#include <pure64/string.h>

#include "misc.h"

#include <stdlib.h>
#include <string.h>

void pure64_dir_init(struct pure64_dir *dir) {
	dir->name_size = 0;
	dir->subdir_count = 0;
	dir->file_count = 0;
	dir->name = NULL;
	dir->subdirs = NULL;
	dir->files = NULL;
}

void pure64_dir_free(struct pure64_dir *dir) {

	pure64_free(dir->name);

	for (uint64_t i = 0; i < dir->subdir_count; i++)
		pure64_dir_free(&dir->subdirs[i]);

	for (uint64_t i = 0; i < dir->file_count; i++)
		pure64_file_free(&dir->files[i]);

	pure64_free(dir->subdirs);
	pure64_free(dir->files);

	dir->name = NULL;
	dir->subdirs = NULL;
	dir->files = NULL;
}

int pure64_dir_add_file(struct pure64_dir *dir, const char *name) {

	int err;
	struct pure64_file *files;
	uint64_t files_size;

	if (pure64_dir_name_exists(dir, name))
		return PURE64_EEXIST;

	files_size = dir->file_count + 1;
	files_size *= sizeof(dir->files[0]);

	files = dir->files;

	files = pure64_realloc(files, files_size);
	if (files == NULL) {
		return PURE64_ENOMEM;
	}

	pure64_file_init(&files[dir->file_count]);

	err = pure64_file_set_name(&files[dir->file_count], name);
	if (err != 0) {
		pure64_file_free(&files[dir->file_count]);
		return err;
	}

	dir->files = files;
	dir->file_count++;

	return 0;
}

int pure64_dir_add_subdir(struct pure64_dir *dir, const char *name) {

	int err;
	struct pure64_dir *subdirs;
	uint64_t subdirs_size;

	if (pure64_dir_name_exists(dir, name))
		return PURE64_EEXIST;

	subdirs_size = dir->subdir_count + 1;
	subdirs_size *= sizeof(dir->subdirs[0]);

	subdirs = dir->subdirs;

	subdirs = pure64_realloc(subdirs, subdirs_size);
	if (subdirs == NULL) {
		return PURE64_ENOMEM;
	}

	pure64_dir_init(&subdirs[dir->subdir_count]);

	err = pure64_dir_set_name(&subdirs[dir->subdir_count], name);
	if (err != 0) {
		pure64_dir_free(&subdirs[dir->subdir_count]);
		return err;
	}

	dir->subdirs = subdirs;
	dir->subdir_count++;

	return 0;
}

int pure64_dir_export(struct pure64_dir *dir, struct pure64_stream *out) {

	int err;

	err = encode_uint64(dir->name_size, out);
	if (err != 0)
		return err;

	err = encode_uint64(dir->subdir_count, out);
	if (err != 0)
		return err;

	err = encode_uint64(dir->file_count, out);
	if (err != 0)
		return err;

	err = pure64_stream_write(out, dir->name, dir->name_size);
	if (err != 0)
		return err;

	for (uint64_t i = 0; i < dir->subdir_count; i++) {
		err = pure64_dir_export(&dir->subdirs[i], out);
		if (err != 0)
			return err;
	}

	for (uint64_t i = 0; i < dir->file_count; i++) {
		err = pure64_file_export(&dir->files[i], out);
		if (err != 0)
			return err;
	}

	return 0;
}

int pure64_dir_import(struct pure64_dir *dir, struct pure64_stream *in) {

	int err;

	err = decode_uint64(&dir->name_size, in);
	if (err != 0)
		return err;

	err = decode_uint64(&dir->subdir_count, in);
	if (err != 0)
		return err;

	err = decode_uint64(&dir->file_count, in);
	if (err != 0)
		return err;

	dir->name = pure64_malloc(dir->name_size + 1);
	dir->subdirs = pure64_malloc(dir->subdir_count * sizeof(dir->subdirs[0]));
	dir->files = pure64_malloc(dir->file_count * sizeof(dir->files[0]));
	if ((dir->name == NULL)
	 || (dir->subdirs == NULL)
	 || (dir->files == NULL)) {
		pure64_free(dir->name);
		pure64_free(dir->subdirs);
		pure64_free(dir->files);
		return PURE64_ENOMEM;
	}

	err = pure64_stream_read(in, dir->name, dir->name_size);
	if (err != 0)
		return err;

	dir->name[dir->name_size] = 0;

	for (uint64_t i = 0; i < dir->subdir_count; i++)
		pure64_dir_init(&dir->subdirs[i]);

	for (uint64_t i = 0; i < dir->file_count; i++)
		pure64_file_init(&dir->files[i]);

	for (uint64_t i = 0; i < dir->subdir_count; i++) {
		err = pure64_dir_import(&dir->subdirs[i], in);
		if (err != 0)
			return err;
	}

	for (uint64_t i = 0; i < dir->file_count; i++) {
		err = pure64_file_import(&dir->files[i], in);
		if (err != 0)
			return err;
	}

	return 0;
}

bool pure64_dir_name_exists(const struct pure64_dir *dir, const char *name) {

	uint64_t i;

	for (i = 0; i < dir->file_count; i++) {
		if (pure64_strcmp(dir->files[i].name, name) == 0)
			return true;
	}

	for (i = 0; i < dir->subdir_count; i++) {
		if (pure64_strcmp(dir->subdirs[i].name, name) == 0)
			return true;
	}

	return false;
}

int pure64_dir_set_name(struct pure64_dir *dir, const char *name) {

	char *tmp_name;
	uint64_t name_size;

	name_size = pure64_strlen(name);

	tmp_name = pure64_malloc(name_size + 1);
	if (tmp_name == NULL)
		return PURE64_ENOMEM;

	pure64_memcpy(tmp_name, name, name_size);

	tmp_name[name_size] = 0;

	pure64_free(dir->name);

	dir->name = tmp_name;
	dir->name_size = name_size;

	return 0;
}
