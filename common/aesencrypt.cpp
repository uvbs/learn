#include "aesencrypt.h"
#include <openssl/aes.h>
#include <assert.h>

namespace DwUtility
{
	namespace encryption
	{
		std::string AESEncrypt(std::string const& in,std::string const& key)
		{
			assert(key.size() == 16);
			size_t s = in.size();
			if ( s%16 != 0 )
			{
				s = 16*(s/16+1);
			}

			std::string strTmp;
			strTmp.resize( s );
			std::string strDes;
			strDes.resize( s );
			std::fill(strTmp.begin(),strTmp.end(),0);
			std::fill(strDes.begin(),strDes.end(),0);
			std::copy( in.begin(),in.end(),strTmp.begin() );

			AES_KEY k;
			AES_set_encrypt_key( (unsigned char*)key.data(),128,&k );
			unsigned char* pSrc = (unsigned char*)strTmp.data();
			unsigned char* pDes = (unsigned char*)strDes.data();
			for ( size_t i = 0;i < s/16;i++ )
			{
				AES_encrypt( pSrc,pDes,&k );
				pSrc += 16;
				pDes += 16;
			}
			return strDes;
		}

		std::string AESDecrypt(std::string const& in,std::string const& key)
		{
			assert(key.size() == 16);
			size_t s = in.size();
			if(s%16!=0)
				return std::string();

			std::string strDes;
			strDes.resize( s );
			std::fill(strDes.begin(),strDes.end(),0);

			AES_KEY k;
			AES_set_decrypt_key( (unsigned char*)key.data(),128,&k );
			unsigned char* pSrc = (unsigned char*)in.data();
			unsigned char* pDes = (unsigned char*)strDes.data();
			for ( size_t i = 0;i < s/16;i++ )
			{
				AES_decrypt( pSrc,pDes,&k );
				pSrc += 16;
				pDes += 16;
			}
			return strDes;
		}
	}
}
