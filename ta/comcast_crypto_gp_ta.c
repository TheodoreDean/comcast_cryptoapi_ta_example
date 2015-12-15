/*
 * Copyright (c) 2014, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * liABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define STR_TRACE_USER_TA "COMCAST_CRYPTO_TA"
#include "string.h"

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include "comcast_crypto_gp_ta.h"

#define DEBUG

static void dump_hash(uint8_t *hash, size_t len)
{
#ifdef DEBUG
	size_t i;
	for (i = 0; i < len; i++)
		DMSG("%02x", hash[i]);
#else
	(void)hash;
	(void)len;
#endif
}

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param  params[4], void **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	DMSG("has been called (session opened with the Comcast Crypto TA)");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	DMSG("has been called (session with the Comcast Crypto TA will be closed)");
}

static TEE_Result call_gp_sha1_interface(uint32_t param_types,
					 TEE_Param params[4])
{
	TEE_OperationHandle operation = NULL;
	TEE_Result res = TEE_ERROR_GENERIC;

	/* Temporary variable for the input (message to be hashed) */
	void *message = NULL;
	size_t message_len = 0;

	/* Temporary variables for the digest */
	void *digest = NULL;
	size_t digest_len = 0;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						   TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	DMSG("Calling GP Internal API for doing SHA1 hashing");
	message = params[0].memref.buffer;
	message_len = params[0].memref.size;

	digest = params[1].memref.buffer;
	digest_len = params[1].memref.size;


	memset(digest, 0, digest_len);

	if (param_types != exp_param_types || !message || !digest)
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_AllocateOperation(&operation, TEE_ALG_SHA1, TEE_MODE_DIGEST, 0);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_AllocateOperation failed! res: 0x%x", res);
		goto out;
	}

	DMSG("message to hash: %s", (char *)message);
	/* Could (should?) be called, raised the question with GP */
	/* TEE_DigestUpdate(operation, (void *)text, message_len); */

	res = TEE_DigestDoFinal(operation, message, message_len, digest,
				&digest_len);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_DigestDoFinal failed! res: 0x%x", res);
		goto out;
	}

	dump_hash(digest, digest_len);
out:
	if (operation)
		TEE_FreeOperation(operation);

	return res;
}

static TEE_Result call_gp_sha256_interface(uint32_t param_types,
					   TEE_Param params[4])
{
	TEE_OperationHandle operation = NULL;
	TEE_Result res = TEE_ERROR_GENERIC;

	/* Temporary variable for the input (message to be hashed) */
	void *message = NULL;
	size_t message_len = 0;

	/* Temporary variables for the digest */
	void *digest = NULL;
	size_t digest_len = 0;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						   TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	DMSG("Calling GP Internal API for doing SHA256 hashing");
	message = params[0].memref.buffer;
	message_len = params[0].memref.size;

	digest = params[1].memref.buffer;
	digest_len = params[1].memref.size;


	memset(digest, 0, digest_len);

	if (param_types != exp_param_types || !message || !digest)
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_AllocateOperation(&operation, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_AllocateOperation failed! res: 0x%x", res);
		goto out;
	}

	DMSG("message to hash: %s", (char *)message);
	/* Could (should?) be called, raised the question with GP */
	/* TEE_DigestUpdate(operation, (void *)text, message_len); */

	res = TEE_DigestDoFinal(operation, message, message_len, digest,
				&digest_len);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_DigestDoFinal failed! res: 0x%x", res);
		goto out;
	}

	dump_hash(digest, digest_len);
out:
	if (operation)
		TEE_FreeOperation(operation);

	return res;
}

static TEE_Result call_gp_corner_cases(uint32_t param_types,
				       TEE_Param params[4])
{
	TEE_OperationHandle operation = NULL;
	TEE_Result res = TEE_ERROR_GENERIC;

	/* Temporary variable for the input (message to be hashed) */
	void *message = NULL;
	size_t message_len = 0;

	/* Temporary variables for the digest */
	void *digest = NULL;
	size_t digest_len = 0;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						   TEE_PARAM_TYPE_MEMREF_OUTPUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	size_t digest_ctr = 0;
	size_t final_ctr = 0;

	DMSG("Running some corner cases");
	message = params[0].memref.buffer;
	message_len = params[0].memref.size;
	(void)message_len;

	digest = params[1].memref.buffer;
	digest_len = params[1].memref.size;


	memset(digest, 0, digest_len);

	if (param_types != exp_param_types || !message || !digest)
		return TEE_ERROR_BAD_PARAMETERS;

	res = TEE_AllocateOperation(&operation, TEE_ALG_SHA1, TEE_MODE_DIGEST, 0);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_AllocateOperation failed! res: 0x%x", res);
		goto out;
	}

	/* 01. Testing all combinations using NULL or valid message */
	for (digest_ctr = 0; digest_ctr < 2; digest_ctr++) {
		for (final_ctr = 0; final_ctr < 2; final_ctr++) {
			DMSG("Looping: D: %d   F: %d", digest_ctr, final_ctr);
			if (digest_ctr)
				TEE_DigestUpdate(operation, message, message_len);
			else
				TEE_DigestUpdate(operation, NULL, 0);

			if (res != TEE_SUCCESS)
				DMSG("TEE_DigestUpdate failed! res: 0x%x, digest_ctr: %d, final_ctr %d", res, digest_ctr, final_ctr);

			if (final_ctr)
				res = TEE_DigestDoFinal(operation, message, message_len, digest,
							&digest_len);
			else
				res = TEE_DigestDoFinal(operation, NULL, 0, digest,
							&digest_len);

			if (res != TEE_SUCCESS)
				DMSG("TEE_DigestDoFinal failed! res: 0x%x, digest: %d, final_ctr %d", res, digest_ctr, final_ctr);
			else
				dump_hash(digest, digest_len);

			/*
			 * Must reset the operation between the different tests.
			 */
			TEE_ResetOperation(operation);
		}
	}

	/* 02. Only calling the final function using NULL message */
	res = TEE_DigestDoFinal(operation, NULL, 0, digest,
				&digest_len);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_DigestDoFinal failed! res: 0x%x", res);
		goto out;
	} else {
		DMSG("02. Only final function, null message");
		dump_hash(digest, digest_len);
	}

	/* 03. Only calling the final function using abc message */
	TEE_ResetOperation(operation);
	res = TEE_DigestDoFinal(operation, message, message_len, digest,
				&digest_len);

	if (res != TEE_SUCCESS) {
		DMSG("TEE_DigestDoFinal failed! res: 0x%x", res);
		goto out;
	} else {
		DMSG("03. Only final function, abc message");
		dump_hash(digest, digest_len);
	}

out:
	if (operation)
		TEE_FreeOperation(operation);

	return res;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	/*
	 * In reality, there is no reason for having two functions here when
	 * just doing hashing. You could just pass the cmd_id to a generic hash
	 * function and in that function choose the hashing algorithm based on
	 * the cmd_id. However, to make this example TA easier to understand we
	 * have added separate functions (almost identical) for the hashing,
	 * i.e, one for SHA1 and one for SHA256.
	 */
	switch (cmd_id) {
	case TAF_SHA1:
		return call_gp_sha1_interface(param_types, params);
	case TAF_SHA256:
		return call_gp_sha256_interface(param_types, params);
	case TAF_CORNER_CASES:
		return call_gp_corner_cases(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
