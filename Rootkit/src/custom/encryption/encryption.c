//this file is frankly a mess and doesnt really work, so ignore this file!!!
#include <linux/string.h>
#include <uapi/linux/random.h>

#include <crypto/blake2b.h>
#include <crypto/aes.h>
#include <custom/encryption/keys.c>


static struct PubPrivKeys{
	struct key_t pubkey;
	struct key_t privkey;
};

static struct AuthKeys{
	struct PubPrivKeys pubpriv_keys_1;
	struct PubPrivKeys pubpriv_keys_2;

	struct key_t client_pubkey_1;
	struct key_t client_pubkey_2;
	struct key_t base;
	struct key_t shared_secret;
	//modulus is 2^512 or size of key_t
};

struct AuthKeys generate_auth_keys(void){
	struct AuthKeys keys;
	uint64_t base[8] = {(uint64_t) 1719739712519262, (uint64_t) 2624592722789091, (uint64_t) 72801792822719, (uint64_t) 8265528192920914,
		(uint64_t) 27160182523510272, (uint64_t) 8172520241715, (uint64_t) 28292782525261, (uint64_t) 81903735271719};
	keys.base = generate_key(base);
	keys.shared_secret = generate_key(null_key);

	keys.client_pubkey_1 = generate_key(null_key);
	keys.client_pubkey_2 = generate_key(null_key);

	keys.pubpriv_keys_1.privkey = generate_key(null_key);
	keys.pubpriv_keys_1.pubkey = generate_key(null_key);
	keys.pubpriv_keys_2.privkey = generate_key(null_key);
	keys.pubpriv_keys_2.pubkey = generate_key(null_key);

	rand_key(&(keys.pubpriv_keys_1.privkey)); //random key
	update_hex(&(keys.pubpriv_keys_1.privkey)); //updates the hex
	keys.pubpriv_keys_1.pubkey = pow_keys(keys.base, keys.pubpriv_keys_1.privkey); // pub = base^priv
	update_hex(&(keys.pubpriv_keys_1.pubkey));

	rand_key(&(keys.pubpriv_keys_2.privkey));
	update_hex(&(keys.pubpriv_keys_2.privkey));
	keys.pubpriv_keys_2.pubkey = pow_keys(keys.base, keys.pubpriv_keys_2.privkey); // pub = base^priv
	update_hex(&(keys.pubpriv_keys_2.pubkey));
	return keys;
}

int menezes_qu_vanstone(struct AuthKeys *keys){
	/*
		x and y are longterm
		(B x Y^B)^(a + xA)
		(client_pubkey_2 * client_pubkey_1^client_pubkey_2 )^(pubpriv_keys_1.privkey + pubpriv_keys_2.privkey*pubpriv_keys_1.pubkey)
	*/
	struct key_t temp1=generate_key(null_key);
	struct key_t temp2=generate_key(null_key);

	temp1 = pow_keys(keys->client_pubkey_1, keys->client_pubkey_2); //client_pubkey_1^client_pubkey_2
	temp1 = multiply_keys(keys->client_pubkey_2, temp1); //client_pubkey_2 * client_pubkey_1^client_pubkey_2

	temp2 = multiply_keys(keys->pubpriv_keys_2.privkey, keys->pubpriv_keys_1.pubkey);
	temp2 = add_keys(keys->pubpriv_keys_1.privkey, temp2);

	keys->shared_secret =  pow_keys(temp1, temp2);
	return 0;
}
