/**
 * DLLʹ�á�������Unix��Win32ƽ̨ͨ��
 *
 * ʹ�÷�����
 *    HINSTANCE hLib;                  //DLL���
 *    void (*dll_bar)(const char* s);; //DLL�еĺ���ԭ�ͣ���ʵ�ʵ�DLL�еĺ���Ϊ׼
 *
 *    // ����DLL
 *    hLib = LoadLibrary("DLL����");
 *    if (hLib == NULL)
 *    {
 *         //����DLLʧ��
 *    }
 *
 *    dll_bar = (void (*)(const char*))GetProcAddress(hLib, "������");
 *    if (dll_bar == NULL)
 *    {
 *         //ȡ������ַʧ��
 *    }
 *    
 *    // ���ú���
 *    dll_bar("string");
 *
 *    // ж��DLL
 *    FreeLibrary(hLib);
 *
 * Xu Lubing
 * Dec. 23, 2003
 */
#ifndef DLL_CALL_H
#define DLL_CALL_H

#ifndef WIN32
/* Unixƽ̨ */
typedef void* HINSTANCE;
typedef void* HMODULE;
#else
/* Win32ƽ̨ */
#include <windows.h>
#endif /* WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32
/**
 * ����DLL
 * ����:
 *   lpLibFileName: DLL������
 * ����:
 *   NULL: ���ظ�����DLLʧ��
 *   ��NULL: ����DLL�ɹ�������ֵ���DLL�ľ��
 */
HINSTANCE LoadLibrary(const char* lpLibFileName);

/**
 * ���DLL��ĳ��������ָ��
 * ����:
 *   hModule: ����LoadLibrary���ص�DLL���
 *   lpProcName: ��Ҫ��ȡ�ĺ���������
 * ����:
 *   NULL: ȡ����ָ��ʧ��
 *   ��NULL: �ɹ�������ֵ�������ĺ���ָ��
 */
void* GetProcAddress(HINSTANCE hModule, const char* lpProcName);

/**
 * ж��DLL
 * ����:
 *   hModule: ����LoadLibrary���ص�DLL���
 * ����:
 *   0: ʧ��
 *   ��0: ж�سɹ�
 */
int FreeLibrary(HINSTANCE hLibModule);
#endif

/**
 * ȡ��һ�ε���DLL�����Ĵ�����Ϣ
 * ����:
 *   lpErrMsg: ���ڴ�Ŵ�����Ϣ
 * [ע��] ���øú�����õĴ�����Ϣ������ص���FreeErrMsg�ͷ�
 */
void LastDLLError(char** lpErrMsg);

/**
 * �ͷŴ�����Ϣ
 * ������
 *   lpErrMsg: ����LastDLLError�õ��Ĵ�����Ϣ
 */
void FreeErrMsg(char* lpErrMsg);

#ifdef __cplusplus
}
#endif

#endif
