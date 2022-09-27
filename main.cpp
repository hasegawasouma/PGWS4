#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include <vector>
#ifdef _DEBUG
#include<iostream>
#endif

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace std;

//@brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
//@param format �t�H�[�}�b�g(%d�Ƃ�%f�Ƃ���)
//@param �ϒ�����
//@remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���B
void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif // _DEBUG
}

//�ʓ|�����Ǐ����Ȃ���΂����Ȃ��֐�
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//����̏������s��
}

#ifdef _DEBUG
int main()
{
#else �A�N�e�B�u�łȂ��v���v���Z�b�T�@�u���b�N
#endif
const unsigned int window_width = 1280;
const unsigned int window_height = 720;

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapchain = nullptr;

//�E�B���h�E�N���X�̍쐬���o�^
WNDCLASSEX w = {};

w.cbSize = sizeof(WNDCLASSEX);
w.lpfnWndProc = (WNDPROC)WindowProcedure;//�R�[���o�b�N�֐��̎w��
w.lpszClassName = _T("DX12Sample");//�A�v���P�[�V�����N���X��
w.hInstance = GetModuleHandle(nullptr);//�n���h���̎擾

RegisterClassEx(&w);//�A�v���P�[�V�����N���X�i�E�B���h�E�N���X��OS�ɓ`����j

RECT wrc = { 0,0,window_width,window_height };//�E�B���h�E�T�C�Y�����߂�

//�֐����g���ăE�B���h�E�̃T�C�Y��␳����
AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
//�E�B���h�E�I�u�W�F�N�g�̐���
HWND hwnd = CreateWindow(w.lpszClassName,
	_T("DX12�e�X�g"),
	WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT,
	CW_USEDEFAULT,
	wrc.right - wrc.left,
	wrc.bottom - wrc.top,
	nullptr,
	nullptr,
	w.hInstance,
	nullptr);//�ǉ��p�����[�^�[

HRESULT D3D12CreateDevice(
	IUnknown * pAdapter,//�ЂƂ܂���nullptr��OK
	D3D_FEATURE_LEVEL MnimumFeatureLevel,//�Œ���K�v�ȃt�B�[�`���[���x��
	REFIID riid,
	void** ppDevice
);

D3D_FEATURE_LEVEL levels[] =
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
};

auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
//�A�_�v�^�[�̗񋓗p
std::vector<IDXGIAdapter*>adapters;
//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
IDXGIAdapter* tmpAdapter = nullptr;
for (auto adpt : adapters)
{
	DXGI_ADAPTER_DESC adesc = {};
	adpt->GetDesc(&adesc);//�A�_�v�^�[�����I�u�W�F�N�g�̎擾
	std::wstring strDesc = adesc.Description;
	//�T�������A�_�v�^�[�̖��O���m�F
	if (strDesc.find(L"NVIDIA") != std::string::npos)
	{
		tmpAdapter = adpt;
		break;
	}
}

//DirectX�f�o�C�X�̏�����
D3D_FEATURE_LEVEL featureLevel;
for (auto lv : levels)
{
	if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
	{
		featureLevel = lv;
		break;//�����\�ȃo�[�W���������������烋�[�v��ł��؂�
	}
}

result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
	IID_PPV_ARGS(&_cmdAllocator));
result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
	_cmdAllocator, nullptr,
	IID_PPV_ARGS(&_cmdList));

D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
//�^�C���A�E�g�Ȃ�
cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//�A�_�v�^�[���P�����g��Ȃ��Ƃ��͂O�ł悢
cmdQueueDesc.NodeMask = 0;
//�v���C�I���e�B�͓��Ɏw��Ȃ�
cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
//�R�}���h���X�g�ƍ��킹��
cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//�L���[�쐬
result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
swapchainDesc.Width = window_width;
swapchainDesc.Height = window_height;
swapchainDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
swapchainDesc.Stereo = false;
swapchainDesc.SampleDesc.Count = 1;
swapchainDesc.SampleDesc.Quality = 0;
swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
swapchainDesc.BufferCount = 2;
//�o�b�N�o�b�t�@�[�͐L�яk�݉\
swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
//�t���b�v��͑��₩�ɂ͔j��
swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//���Ɏw��Ȃ�
swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
//�E�B���h�E�A�t���X�N���[���̐؂�ւ��\
swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
result = _dxgiFactory->CreateSwapChainForHwnd(
	_cmdQueue, hwnd,
	&swapchainDesc, nullptr, nullptr,
	(IDXGISwapChain1**)&_swapchain);//�{����QueryInterface����p����
//IDXGISwapChain4*�ւ̕ϊ��`�F�b�N�����邪�A
//�����ł͂킩��₷���d�����߂ɃL���X�g�őΉ�

D3D12_DESCRIPTOR_HEAP_DESC hespDesc = {};
hespDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
hespDesc.NodeMask = 0;
hespDesc.NumDescriptors = 2;//�\���̂Q��
hespDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//���Ɏw��Ȃ�

ID3D12DescriptorHeap* rtvHeaps = nullptr;
result = _dev->CreateDescriptorHeap(&hespDesc, IID_PPV_ARGS(&rtvHeaps));

DXGI_SWAP_CHAIN_DESC swcDesc = {};
result = _swapchain->GetDesc(&swcDesc);

std::vector<ID3D12Resource*>_backBuffers(swcDesc.BufferCount);
for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
{
	result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
	D3D12_CPU_DESCRIPTOR_HANDLE handle
		= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
}

//�E�B���h�E�\��
ShowWindow(hwnd, SW_SHOW);

while (true)
{
	MSG msg;
	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {

		//�A�v���P�[�V�������I���Ƃ���messeage��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	//DirectX����
		//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	//�����_�[�^�[�Q�b�g���w��
	auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	//��ʃN���A
	float clearcolor[] = { 1.0f,1.0f,0.0f,1.0f };//���F
	_cmdList->ClearRenderTargetView(rtvH, clearcolor, 0, nullptr);

	//���߂̃N���[�Y
	_cmdList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _cmdList };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);

	_cmdAllocator->Reset();//�L���[���N���A
	_cmdList->Reset(_cmdAllocator, nullptr);//�ĂуR�}���h���X�g�����߂鏀��

	//�t���b�v
	_swapchain->Present(1, 0);
}

//�����N���X�͎g��Ȃ��̂œo�^��������
UnregisterClass(w.lpszClassName, w.hInstance);

return 0;
}
