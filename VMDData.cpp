#include "pch.h"
#include "VMDData.h"

VMDData::VMDData(std::string strVMDPath)
	:duration(0)
{
	LoadVMDData(strVMDPath);
	SetMotionDatas();
}

//VMDデータ読み込み
void VMDData::LoadVMDData(std::string strVMDPath)
{
	FILE* fp;
	auto err = fopen_s(&fp, strVMDPath.c_str(), "rb");

	fseek(fp, 50, SEEK_SET);

	motionDataNum = 0;
	fread(&motionDataNum, sizeof(motionDataNum), 1, fp);

	vmdMotionDatas.resize(motionDataNum);
	for (auto& motion : vmdMotionDatas)
	{
		fread(motion.boneName, sizeof(motion.boneName), 1, fp);
		fread(&motion.frameNo, sizeof(motion.frameNo)
			+ sizeof(motion.location)
			+ sizeof(motion.quaternion)
			+ sizeof(motion.bezier),
			1,
			fp);

	}


	fclose(fp);
}

void VMDData::SetMotionDatas()
{

	for (auto& vmdMotion : vmdMotionDatas)
	{
		//クォータニオン、コントロールポイントを一次変数に格納
		auto q = DirectX::XMLoadFloat4(&vmdMotion.quaternion);
		auto cp1 = DirectX::XMFLOAT2(static_cast<float>(vmdMotion.bezier[3]) / 127.0f,
			static_cast<float>(vmdMotion.bezier[7]) / 127.0f );
		auto cp2 = DirectX::XMFLOAT2(static_cast<float>(vmdMotion.bezier[11]) / 127.0f,
			static_cast<float>(vmdMotion.bezier[15]) / 127.0f);

		motionDatas[vmdMotion.boneName].emplace_back(
			Motion(vmdMotion.frameNo, q, cp1, cp2)
		);

		duration = std::max<unsigned int>(duration, vmdMotion.frameNo);
	}

	//フレーム番号でソートする
	auto isLessThanFrameNo = [](const Motion& leftval, const Motion& rightval)
	{
		return leftval.frameNo <= rightval.frameNo;
	};
	for (auto& motion : motionDatas)
	{
		std::sort(motion.second.begin(), motion.second.end(), isLessThanFrameNo);
	}

}
