#include "pch.h"
#include "VMDData.h"

VMDData::VMDData(std::string strVMDPath)
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
		auto q = DirectX::XMLoadFloat4(&vmdMotion.quaternion);
		motionDatas[vmdMotion.boneName].emplace_back(
			Motion(vmdMotion.frameNo, q)
		);

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
