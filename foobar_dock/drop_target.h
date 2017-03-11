#ifndef __DROP_TARGET__H
#define __DROP_TARGET__H

class DropTarget : public IDropTarget
{
	HRESULT DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT DragLeave();
	HRESULT Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
};


#endif
