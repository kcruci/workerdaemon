#include "csconv.h"

#ifdef __THREAD_SAFE__
#include "guard_lock.h"
#endif

template <class _CS1, class _CS2>
static int csconv(iconv_t tID, const _CS1* pcs1, size_t nlen1, _CS2* pcs2, size_t nlen2)
{
	size_t nleft1 = nlen1*sizeof(_CS1);
	size_t nleft2 = nlen2*sizeof(_CS2);
	char* cpcs1 = (char*)pcs1;
	char* cpcs2 = (char*)pcs2;
	size_t nConv = iconv(tID, &cpcs1, &nleft1, &cpcs2, &nleft2);
	if (nConv==(size_t)-1)
	{
		return -1;
	}
	return (nlen2-nleft2/sizeof(_CS2));
}

XGBUC::XGBUC()
{
	m_tID = iconv_open("UCS-2", "GB18030");
}
XGBUC::~XGBUC()
{
	iconv_close(m_tID);
}
int XGBUC::conv(const T_GB* pgb, size_t ngblen, T_UC* puc, size_t nuclen)
{
	return csconv(m_tID, pgb, ngblen, puc, nuclen);
}
int XGBUC::cv(const T_GB* pgb, size_t ngblen, T_UC* puc, size_t nuclen)
{
	#ifdef __THREAD_SAFE__
		CGuardLock lock(mutex_);
	#endif

	static XGBUC s_oGbUc;
	return s_oGbUc.conv(pgb, ngblen, puc, nuclen);
}

XUCGB::XUCGB()
{
	m_tID = iconv_open("GB18030", "UCS-2");
}
XUCGB::~XUCGB()
{
	iconv_close(m_tID);
}
int XUCGB::conv(const T_UC* puc, size_t nuclen, T_GB* pgb, size_t ngblen)
{
	return csconv(m_tID, puc, nuclen, pgb, ngblen);
}
int XUCGB::cv(const T_UC* puc, size_t nuclen, T_GB* pgb, size_t ngblen)
{
	#ifdef __THREAD_SAFE__
		CGuardLock lock(mutex_);
	#endif

	static XUCGB s_oUcGb;
	return s_oUcGb.conv(puc, nuclen, pgb, ngblen);
}

int XUCUTF8::cv(const T_UC* puc, size_t nuclen, T_UTF8* putf8, size_t nutf8len)
{
	const T_UC* ucbpos = puc;
	const T_UC* ucepos = puc+nuclen;
	T_UTF8* utf8bpos = putf8;
	T_UTF8* utf8epos = putf8+nutf8len;
	while (ucbpos< ucepos && utf8bpos<utf8epos)
	{
		if (*ucbpos < 0x80)
		{
			*utf8bpos++ = *ucbpos++;
		}
		else if (*ucbpos < 0x800)
		{
			if (utf8epos-utf8bpos < 2)
			{
				break;
			}
			*utf8bpos++ = ((*ucbpos&0x7C0)>>6) | 0xC0;
			*utf8bpos++ = (*ucbpos++ & 0x3F) | 0x80;
		}
		else
		{
			if (utf8epos-utf8bpos < 3)
			{
				break;
			}
			*utf8bpos++ = ((*ucbpos&0xF000)>>12) | 0xE0;
			*utf8bpos++ = ((*ucbpos&0x0FC0)>>6) | 0x80;
			*utf8bpos++ = ((*ucbpos++&0x3F)) | 0x80;
		}
	}
	return (utf8bpos-putf8);
}
