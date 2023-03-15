#include "pch.h"
#include "PmdBone.h"

using namespace::DirectX;

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
		if (boneNames[idx].find("�Ђ�") != std::string::npos)
		{
			kneeIdxes.emplace_back(idx);
		}

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

	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());


}

//�t�H���[�h�L�l�}�e�B�N�X�̎�������
//�t���[�������ăL�[�t���[���𔭌��A�O�L�[�t���[���ƕ�Ԃ���]�s��+���s�ړ��s������肷��B
//TODO: ���̊֐��͕����������B
void PmdBone::SetBoneMatrices(unsigned int frameNo)
{
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());



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
		auto isMoreThanPredicate = [frameNo](const Motion& motion) {
			return motion.frameNo <= frameNo;
		};

		//�߂��l�̃C�e���[�^�[�𓾂�A�����ɕ���ł���f�[�^��T������̂Ń��o�[�X�C�e���[�^�[���g��
		auto rit = std::find_if(
			motions.rbegin(), motions.rend(), isMoreThanPredicate
		);

		//���[�V�����f�[�^���Ȃ��Ȃ�continue
		if (rit == motions.rend())
		{
			continue;
		}

		//�O�L�[�t���[���Ǝ��L�[�t���[���ŕ�Ԃ��s��
		//��]�f�[�^(quaternion)�ƃ��P�[�V�����f�[�^(offset)�ōs����v�Z����
		XMMATRIX rotation;
		XMVECTOR offset = XMLoadFloat3(&rit->offset);
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			//�R���g���[���|�C���g����x�W�F�Ȑ����v�Z���A��ԌW�����v�Z
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->controlPoint1, it->controlPoint2, 12);

			//���`��Ԃ��v�Z
			rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			offset = XMVectorLerp(offset, XMLoadFloat3(&it->offset), t);
		}

		//�����s����i�[�B�Z���^�[����ċA�œ`��������B
		//��]�͌��_�ōs��
		auto& pos = node.startPos;
		auto matrix = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix * XMMatrixTranslationFromVector(offset);
	}

	RecursiveMatrixMultiply(&boneNodeTable["�Z���^�["], XMMatrixIdentity());


}


void PmdBone::RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat)
{
	//�i�[����Ă���P�ʍs��ɓ����ϊ��s����|���A�f�[�^��ۑ�
	boneMatrices[node->boneIdx] *= mat;

	for (auto& childNode : node->children)
	{
		RecursiveMatrixMultiply(childNode, boneMatrices[node->boneIdx] );
	}
}

float PmdBone::GetYFromXOnBezier(float x, const XMFLOAT2& controlPoint1, const XMFLOAT2& controlPoint2, uint8_t max_steps)
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

void PmdBone::IKSolve(uint32_t frameNo)
{
	for (auto& ik : motionIKs)
	{
		//IK�X�C�b�`�`�F�b�N
		if (isOnIkInFrame(frameNo, ik) == false)
		{
			continue;
		}

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
	//�ŏI�^�[�Q�b�g�̈ʒu���i�[
	auto targetBoneNode = boneNodeAddressArray[ik.boneIdx];
	auto targetOriginPos = XMLoadFloat3(&targetBoneNode->startPos);

	auto parentMat = boneMatrices[boneNodeAddressArray[ik.boneIdx]->ikParentBone];
	//�ꎞ�I��IK�̐e�̉�]�s��𖳎����邽�ߋt�s����v�Z���K�p
	XMVECTOR determinant;
	auto inverseParentMat = XMMatrixInverse(&determinant, parentMat);
	auto targetNextPos = XMVector3Transform(targetOriginPos, boneMatrices[ik.boneIdx] * inverseParentMat);

	//���[�m�[�h�ʒu
	auto endPos = XMLoadFloat3(&boneNodeAddressArray[ik.targetIdx]->startPos);

	//CCD-IK�p�̊e�m�[�h�̈ʒu�A��]�s����i�[
	std::vector<XMVECTOR> positions;
	std::vector<XMMATRIX> matrices;
	matrices.resize(ik.nodeIdx.size());
	for (auto& nodeId : ik.nodeIdx)
	{
		positions.emplace_back(XMLoadFloat3(&boneNodeAddressArray[nodeId]->startPos));
	}
	fill(matrices.begin(), matrices.end(), XMMatrixIdentity());

	//���e�덷
	float epsilon = 0.005f;

	//CCD-IK�{��
	for (int c = 0; c < ik.iterations; ++c)
	{
		if (XMVector3Length(XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
		{
			break;
		}

		//�p�x�������l�����A�m�[�h�������̂ڂ�{�[�����Ȃ��Ă���
		for (int nodeId = 0; nodeId < positions.size(); ++nodeId)
		{
			const auto& pos = positions[nodeId];

			//���݂̃m�[�h�Ɩ��[�m�[�h�A�ړI�ʒu�ւ̃x�N�g�����Ƃ�
			auto vectorToEnd = XMVectorSubtract(endPos, pos);
			auto vectorToTarget = XMVectorSubtract(targetNextPos, pos);
			vectorToEnd = XMVector3Normalize(vectorToEnd);
			vectorToTarget = XMVector3Normalize(vectorToTarget);

			//��̃x�N�g���������Ȃ�ł��邱�Ƃ͖������ߎ��̃m�[�h�w
			if (XMVector3Length(XMVectorSubtract(vectorToEnd, vectorToTarget)).m128_f32[0] <= epsilon)
			{
				continue;
			}

			//���W���ƃx�N�g���Ԋp�x�𓾂�
			auto cross = XMVector3Normalize(XMVector3Cross(vectorToEnd, vectorToTarget));
			float angle = XMVector2AngleBetweenVectors(vectorToEnd, vectorToTarget).m128_f32[0];

			//��]�p�������l��
			angle = min(angle, ik.limit);
			XMMATRIX rotation = XMMatrixRotationAxis(cross, angle);

			//pos���S�ɉ�]?
			auto matrix = XMMatrixTranslationFromVector(-pos)
				* rotation
				* XMMatrixTranslationFromVector(pos);

			//�s����|�������ŏI�I�ȉ�]�s������
			matrices[nodeId] *= matrix;

			//��������[���ɂ���_��S�č���v�Z�����s��ŉ�]����B
			for (auto id = nodeId; id >= 0; --id)
			{
				positions[id] = XMVector3Transform(positions[id], matrix);
			}
			endPos = XMVector3Transform(endPos, matrix);


			//���e�덷�ȓ��Ȃ�I��
			if (XMVector3Length(XMVectorSubtract(endPos, targetNextPos)).m128_f32[0] <= epsilon)
			{
				break;
			}
		}
	}

	//CCD-IK�I����Ƀ{�[���N���X�̉�]�s��ɔ��f
	int id = 0;
	for (auto& nodeId : ik.nodeIdx)
	{
		boneMatrices[nodeId] = matrices[id];
		id++;
	}

	//���������e�̍s���߂��Čv�Z
	auto rootNode = boneNodeAddressArray[ik.nodeIdx.back()];
	RecursiveMatrixMultiply(rootNode, parentMat);
}

//�]���藝�ɂ��IK
void PmdBone::SolveCosineIK(const PMDIK& ik)
{
	//�e�m�[�h�̈ʒu�A�y�у{�[���������i�[
	std::vector<XMVECTOR> positions;
	std::array<float, 2> edgeLens;

	//�ŏI�^�[�Q�b�g�ʒu���i�[
	auto& targetNode = boneNodeAddressArray[ik.boneIdx];
	auto targetPos = XMVector3Transform(XMLoadFloat3(&targetNode->startPos), boneMatrices[ik.boneIdx]);

	//���[�{�[���̍��W�i�[
	auto endNode = boneNodeAddressArray[ik.targetIdx];
	positions.emplace_back(XMLoadFloat3(&endNode->startPos));

	//���ԋy�у��[�g�{�[���̍��W�i�[
	for (auto& chainBoneIdx : ik.nodeIdx)
	{
		auto boneNode = boneNodeAddressArray[chainBoneIdx];
		positions.emplace_back(XMLoadFloat3(&boneNode->startPos));
	}

	//�킩��₷���̂��ߋt����
	reverse(positions.begin(), positions.end());

	edgeLens[0] = XMVector3Length(XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
	edgeLens[1] = XMVector3Length(XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

	//���[�g�{�[����FK�œ������݂̍��W�ɕϊ�
	positions[0] = XMVector3Transform(positions[0], boneMatrices[ik.nodeIdx[1]]);
	//��[�{�[����FK�œ������݂̍��W�ɕϊ�
	positions[2] = XMVector3Transform(positions[2], boneMatrices[ik.boneIdx]);

	//���[�g�����[�ւ̃x�N�g���̍쐬
	auto linearVec = XMVectorSubtract(positions[2], positions[0]);

	//�]���藝�̌v�Z�̏���
	float A = XMVector3Length(linearVec).m128_f32[0];
	float B = edgeLens[0];
	float C = edgeLens[1];

	linearVec = XMVector3Normalize(linearVec);
	
	//���[�g�m�[�h�ɂ�����p�̌v�Z
	float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
	//�^�񒆃m�[�h�ɂ�����p�̌v�Z
	float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

	//��]���̐����A�^�񒆂̃m�[�h���G�m�[�h�Ȃ�x������]���Ƃ���
	XMVECTOR axis;
	if (std::find(kneeIdxes.begin(), kneeIdxes.end(), ik.nodeIdx[0]) == kneeIdxes.end())
	{
		//3�̃m�[�h���Ȃ����ʂ̖@���x�N�g������]��
		//XXX : 3�̃m�[�h��������ɕ���ł���ƃN���b�V��
		auto vm = XMVector3Normalize(XMVectorSubtract(positions[2], positions[0]));
		auto vt = XMVector3Normalize(XMVectorSubtract(targetPos, positions[0]));
		axis = XMVector3Cross(vm, vt);
	}
	else
	{
		auto right = XMFLOAT3(1, 0, 0);
		axis = XMLoadFloat3(&right);
	}

	//���_�ɖ߂��Ă����]������
	auto mat1 = XMMatrixTranslationFromVector(-positions[0]);
	mat1 *= XMMatrixRotationAxis(axis, theta1);
	mat1 *= XMMatrixTranslationFromVector(positions[0]);

	//�^�񒆂̃m�[�h�͉�]�̐������t�ƂȂ�̂�-(pi-theta2)�ƂȂ�
	auto mat2 = XMMatrixTranslationFromVector(-positions[1]);
	mat2 *= XMMatrixRotationAxis(axis, -(XM_PI-theta2));
	mat2 *= XMMatrixTranslationFromVector(positions[1]);

	boneMatrices[ik.nodeIdx[1]] *= mat1;
	//���[�g�m�[�h�̌��ʂɂ���ɉ�]�s����|����
	boneMatrices[ik.nodeIdx[0]] = mat2 * boneMatrices[ik.nodeIdx[1]];
	//���[�m�[�h�ɂ���]�s���`��
	boneMatrices[ik.targetIdx] = boneMatrices[ik.nodeIdx[0]];
}

//IK�K�p�O�̃��[�g����^�[�Q�b�g�ւ̃x�N�g����IK�K�p��̃��[�g����^�[�Q�b�g�ւ̃x�N�g���𓾂�
//������LookAtMatrix�œK�p�O�x�N�g������K�p��x�N�g���ւ̉�]�s��𓾂Ċi�[����
void PmdBone::SolveLookAt(const PMDIK& ik)
{
	//�ԓ_�͖������ߍŏ��̃m�[�h�����[�g�m�[�h�ƂȂ�
	auto rootNode = boneNodeAddressArray[ik.nodeIdx[0]];
	auto targetNode = boneNodeAddressArray[ik.boneIdx];
	
	auto rootPos1 = XMLoadFloat3(&rootNode->startPos);
	auto targetPos1 = XMLoadFloat3(&targetNode->startPos);

	auto rootPos2 = XMVector3Transform(rootPos1, boneMatrices[ik.nodeIdx[0]]);
	auto targetPos2 = XMVector3Transform(targetPos1, boneMatrices[ik.boneIdx]);

	auto originVec = XMVectorSubtract(targetPos1, rootPos1);
	auto targetVec = XMVectorSubtract(targetPos2, rootPos2);

	originVec = XMVector3Normalize(originVec);
	targetVec = XMVector3Normalize(targetVec);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 right = XMFLOAT3(1, 0, 0);

	boneMatrices[ik.nodeIdx[0]] = LookAtMatrix(originVec, targetVec, up, right);

}

//z����lookat�Ɍ�����]�s������
//lookat�ƕ⏕�x�N�g���œ��O�ς��s���A���W�n�����Ɛ��K�����ꂽx,y,z������������B������s��̊e��Ɋi�[����Ɖ�]����W��������B
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	//��������������
	XMVECTOR vz = XMVector3Normalize(lookat);

	//����y���x�N�g��
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&up));

	//x���x�N�g��
	XMVECTOR vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	//y���x�N�g��
	vy = XMVector3Normalize(XMVector3Cross(vz, vx));

	//lookat��up�̕����������ꍇright���g���쐬����
	if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f)
	{
		vx = XMVector3Normalize(XMLoadFloat3(&right));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}

	XMMATRIX ret = XMMatrixIdentity();
	ret.r[0] = vx;
	ret.r[1] = vy;
	ret.r[2] = vz;

	return ret;
}

//z����C�ӂ̃x�N�g��(origin)�Ɍ�����s����v�Z�����̋t�s����v�Z����B�܂�origin��z���Ɍ��������]�s������(1)
//(1)��z����lookat�Ɍ��������]�s����|���Aorigin��lookat�Ɍ��������]�s������B
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	return XMMatrixTranspose(LookAtMatrix(origin, up, right))
			* LookAtMatrix(lookat, up, right);
}


// �t���[����IK�X�C�b�`�m�F
bool PmdBone::isOnIkInFrame(uint32_t frameNo, PMDIK& ik)
{
	auto iterator = std::find_if(vmdData.vmdIkEnableDatas.rbegin(), vmdData.vmdIkEnableDatas.rend(),
		[frameNo](const VMDIKEnable& ikEnable)
		{
			return ikEnable.frameNo <= frameNo;
		});

	//IK�X�C�b�`�ɂ���AIK�X�C�b�`��false�Ȃ�false��Ԃ�
	if (iterator != vmdData.vmdIkEnableDatas.rend())
	{
		auto ikEnableIt = iterator->ikEnableTable.find(boneNames[ik.boneIdx]);

		if (ikEnableIt != iterator->ikEnableTable.end())
		{
			if (ikEnableIt->second == false)
			{
				return false;
			}
		}
	}


	return true;
}