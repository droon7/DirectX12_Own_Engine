#include "pch.h"
#include "PmdBone.h"

PmdBone::PmdBone()
{
}

//�{�[���e�[�u���A��]�s��AIK���A���[�V������������������
PmdBone::PmdBone(std::vector<PmdBoneData> pmdBoneDatas, std::vector<PMDIK> pmdIks, std::string motionPath)
{
	CreateBoneNodeTable(pmdBoneDatas);
	InitBoneMatrices(pmdBoneDatas);

	LoadPmdIks(pmdIks);
	vmdData = VMDData(motionPath);
}

//�e�q�֌W�̂���{�[���m�[�h�e�[�u�������
void PmdBone::CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneNames.resize(pmdBoneDatas.size());
	boneNodeAddressArray.resize(pmdBoneDatas.size());

	//�{�[���l�[���Y�ƘA�z�z��ƃm�[�h�A���C�ɑS�{�[���f�[�^������
	for (int idx = 0; idx < pmdBoneDatas.size(); ++idx)
	{
		auto& pmdbonedata = pmdBoneDatas[idx];
		boneNames[idx] = pmdbonedata.boneName;

		auto& node = boneNodeTable[pmdbonedata.boneName];
		node.boneIdx = idx;
		node.startPos = pmdbonedata.pos;
		node.boneType = pmdbonedata.Type;
		node.ikParentBone = pmdbonedata.ikBoneNo;

		boneNodeAddressArray[idx] = &node;
	}

	//�S�Ă̐e�q�֌W�����
	for (auto& pmdbonedata : pmdBoneDatas)
	{
		//����continue
		if (pmdbonedata.parentNo >= pmdBoneDatas.size())
		{
			continue;
		}

		//�C�e���[�^���w���m�[�h�̐e�m�[�h���i�[���A������g���e�m�[�h�̎q�����o�Ɏ�����ǉ�����B
		//�e:�q�̊֌W��1:1-n�ƂȂ�A�t�͂��肦�Ȃ�
		auto parentName = boneNames[pmdbonedata.parentNo];
		boneNodeTable[parentName].children.emplace_back(&boneNodeTable[pmdbonedata.boneName]);
	}

}

void PmdBone::LoadPmdIks(std::vector<PMDIK> pmdIk)
{
	motionIKs.resize(pmdIk.size());

	for (int i = 0; i < pmdIk.size(); ++i)
	{
		motionIKs[i] = pmdIk[i];
	}
}

//�{�[���ϊ��s�񏉊���
void PmdBone::InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneMatrices.resize(pmdBoneDatas.size());

	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());


}

//�t���[�������ăL�[�t���[���𔭌��A�O�L�[�t���[���ƕ�Ԃ���]�s������肷��B
//TODO: ���̊֐��͕����������B
void PmdBone::SetBoneMatrices(unsigned int frameNo)
{
	std::fill(boneMatrices.begin(), boneMatrices.end(), DirectX::XMMatrixIdentity());

	for (auto& bonemotion : vmdData.motionDatas)
	{
		//���_�̃{�[���f�[�^�ƃ��[�V�����̃{�[���f�[�^����v���邱�Ƃ��m�F�A�Ȃ����continue
		auto itBoneNode = boneNodeTable.find(bonemotion.first);
		if (itBoneNode == boneNodeTable.end())
		{
			continue;
		}

		auto node = boneNodeTable[bonemotion.first];
		auto motions = bonemotion.second;

		//�^����ꂽ�t���[���ԍ��ƃ��[�V�����̃t���[���ԍ��������Ă��邩���肷��֐��I�u�W�F�N�g
		auto predicate = [frameNo](const Motion& motion) {
			return motion.frameNo <= frameNo;
		};

		//�߂��l�̃C�e���[�^�[�𓾂�A�����ɕ���ł���f�[�^��T������̂Ń��o�[�X�C�e���[�^�[���g��
		auto rit = std::find_if(
			motions.rbegin(), motions.rend(), predicate
		);

		//���[�V�����f�[�^���Ȃ��Ȃ�continue
		if (rit == motions.rend())
		{
			continue;
		}

		//�O�L�[�t���[���Ǝ��L�[�t���[���ŕ�Ԃ��s��
		DirectX::XMMATRIX rotation;
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = DirectX::XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			//�x�W�F�Ȑ����
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->controlPoint1, it->controlPoint2, 12);
			rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			//rotation = DirectX::XMMatrixRotationQuaternion(rit->quaternion);
		}

		//������]�s����i�[�B�Z���^�[����ċA�œ`��������B
		auto& pos = node.startPos;
		auto matrix = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix;
	}

	RecursiveMatrixMultiply(&boneNodeTable["�Z���^�["], DirectX::XMMatrixIdentity());
}


void PmdBone::RecursiveMatrixMultiply(BoneNode* node, const DirectX::XMMATRIX& mat)
{
	//�i�[����Ă���P�ʍs��ɓ����ϊ��s����|���A�f�[�^��ۑ�
	boneMatrices[node->boneIdx] *= mat;

	for (auto& childNode : node->children)
	{
		RecursiveMatrixMultiply(childNode, boneMatrices[node->boneIdx] );
	}
}

float PmdBone::GetYFromXOnBezier(float x, const DirectX::XMFLOAT2& controlPoint1, const DirectX::XMFLOAT2& controlPoint2, uint8_t max_steps)
{
	if (controlPoint1.x == controlPoint1.y && controlPoint2.x == controlPoint2.y)
	{
		return x;
	}

	float t = x;
	//�W��
	const float k0 = 1 + 3* controlPoint1.x - 3*controlPoint2.x;
	const float k1 = 3 * controlPoint2.x - 6 * controlPoint1.x;
	const float k2 = 3 * controlPoint1.x;

	//����l
	constexpr float epsilon = 0.0005f;

	//�ߎ��l�܂Ōv�Z����
	for (int i = 0; i < max_steps; ++i)
	{

		auto ft = k0*t*t*t + k1*t*t + k2*t - x;

		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}
		t -= ft / 2;
	}

	auto r = 1 - t;
	auto ret = t*t*t + 3*t*t*r*controlPoint2.y + 3*t*r*r* controlPoint1.y;
	
	return ret;
}

void PmdBone::IKSolve()
{
	for (auto& ik : motionIKs)
	{
		auto childrenNodesCount = ik.nodeIdx.size();

		//CCD-IK�͏d���̂Ŋԓ_�����Ȃ���ΊȒP��IK�ŉ���
		switch (childrenNodesCount)
		{
		case 0: //�G���[
			assert(0);
			continue;
		case 1:
			SolveLookAt(ik);
			break;
		case 2:
			SolveCosineIK(ik);
			break;
		default:
			SolveCCDIK(ik);
		}
	}
}

void PmdBone::SolveCCDIK(const PMDIK& ik)
{
}

void PmdBone::SolveCosineIK(const PMDIK& ik)
{
}

void PmdBone::SolveLookAt(const PMDIK& ik)
{
}

//���O�ς��v�Z���A���W���𓾂�LookAt�s���Ԃ�
DirectX::XMMATRIX PmdBone::LookAtMatrix(const DirectX::XMVECTOR& lookat, DirectX::XMFLOAT3& up, DirectX::XMFLOAT3& right)
{
	//��������������
	DirectX::XMVECTOR vz = lookat;

	//����y���x�N�g��
	DirectX::XMVECTOR vy = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&up));

	//x���x�N�g��
	DirectX::XMVECTOR vx = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vy, vz));
	//y���x�N�g��
	vy = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vz, vx));

	//lookat��up�̕����������ꍇright���g���쐬����
	if (std::abs(DirectX::XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f)
	{
		vx = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&right));
		vy = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vz, vx));
		vx = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vy, vz));
	}

	DirectX::XMMATRIX ret = DirectX::XMMatrixIdentity();
	ret.r[0] = vx;
	ret.r[1] = vy;
	ret.r[2] = vz;

	return ret;
}

//z����C�ӂ̃x�N�g��(origin)�Ɍ�����s����v�Z�����̋t�s����v�Z����
//�����t�s���z�������̕���(lookat)�Ɍ�����s����v�Z���A��Z����ƌ��ʂ𓾂���
DirectX::XMMATRIX PmdBone::LookAtMatrix(const DirectX::XMVECTOR& origin, const DirectX::XMVECTOR& lookat, DirectX::XMFLOAT3& up, DirectX::XMFLOAT3& right)
{
	return DirectX::XMMatrixTranspose(LookAtMatrix(origin, up, right))
			* LookAtMatrix(lookat, up, right);
}
