[ComponentEditorProps(category: "GameScripted/Coop", description: "Allows spawning of AI groups.")]
class RLC_AISpawnerComponentClass : ScriptComponentClass
{
}

void ScriptInvoker_OnSpawnerEmptyDelegate();
typedef func ScriptInvoker_OnSpawnerEmptyDelegate;
typedef ScriptInvokerBase<ScriptInvoker_OnSpawnerEmptyDelegate> ScriptInvoker_OnSpawnerEmpty;


//------------------------------------------------------------------------------------------------
class RLC_AISpawnerComponent : ScriptComponent
{
	[Attribute("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", UIWidgets.EditBox, "Group prefab to spawn.")]
	protected ResourceName m_rnDefaultPrefab;

	[Attribute("0", UIWidgets.CheckBox, "If checked, spawns group immediately.")]
	protected bool m_bSpawnImmediately;

	[Attribute("0", UIWidgets.CheckBox, "If checked, respawn when died.")]
	protected bool m_bRespawn;
	
	IEntity Owner;


	vector parentVector[4]
	// Attached component.
	protected RplComponent m_pRplComponent;

	//! Spawned agent relevant to the authority only.
	protected AIAgent m_pSpawnedAgent;
	
	//! Invoker which we can hook onto - see typedef above
	protected ref ScriptInvoker_OnSpawnerEmpty m_pOnEmptyInvoker = new ScriptInvoker_OnSpawnerEmpty();
	
	
	AIAgent GetSpawnedAgent()
	{
		return m_pSpawnedAgent;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker_OnSpawnerEmpty GetOnEmptyInvoker()
	{
		return m_pOnEmptyInvoker;
	}
	
	void RemoveSpawned()
	{
		if(m_pSpawnedAgent)
			RplComponent.DeleteRplEntity(m_pSpawnedAgent, false);
	}
	
	//------------------------------------------------------------------------------------------------
	bool DoSpawn(ResourceName aiAgentPrefab)
	{
		if (IsSpawned())
		{
			Print("RLC_AISpawnerComponent cannot spawn group; group was spawned already!");
			return false;
		}

		if (!VerifyRplComponentPresent())
		{
			Print("RLC_AISpawnerComponent cannot spawn group, spawner has no RplComponent!");
			return false;
		}

		if (!m_pRplComponent.IsMaster())
		{
			Print("RLC_AISpawnerComponent caught non-master request to spawn!");
			return false;
		}

		Resource agentPrefab = Resource.Load(aiAgentPrefab);
		if (!agentPrefab)
		{
			Print(string.Format("RLC_AISpawnerComponent could not load '%1'", agentPrefab));
			return false;
		}

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;	
		spawnParams.Transform = parentVector;	
		

		BaseWorld tmp = GetOwner().GetWorld();
				
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(agentPrefab, tmp, spawnParams);
		if (!spawnedEntity)
		{
			Print(string.Format("RLC_AISpawnerComponent could not spawn '%1'", agentPrefab));
			return false;
		}

		AIAgent agent = AIAgent.Cast(spawnedEntity);
		if (!agent)
		{
			Print(string.Format("RLC_AISpawnerComponent spawned entity '%1' that is not of AIAgent type, deleting!", agentPrefab));
			RplComponent.DeleteRplEntity(spawnedEntity, false);
			return false;
		}

		// Store agent
		m_pSpawnedAgent = agent;

		BaseWorld world = GetOwner().GetWorld();
		
		
		ref array<IEntity> children = new array<IEntity>();
		RLC_Statics.GetAllChildren(Owner,children);

		
		
		AIWaypointCycle cycle;
		SCR_DefendWaypoint defend;
		SCR_BoardingWaypoint onBoard;
		
		if(!children || children.Count()<=0) 
		{
			IEntity parent = Owner.GetParent();
			if(!parent)
			{
				Debug.Error("RLC_AISpawnerComponent cannot hook to the waypoint, it is not a child of SCR_DefendWaypoint!");
				return false;
			}
			defend = SCR_DefendWaypoint.Cast(SCR_DefendWaypoint.Cast(parent));
			onBoard = SCR_BoardingWaypoint.Cast(SCR_BoardingWaypoint.Cast(parent));
			if(!defend && !onBoard)
			{
				Debug.Error("RLC_AISpawnerComponent cannot hook to the waypoint, it is not a child of SCR_DefendWaypoint nor a SCR_BoardingWaypoint!");
				return false;
			}
			
		}
		
		if(!defend && !onBoard)
		{
			ref array<AIWaypoint> patrolWaypoints = new array<AIWaypoint>();
			foreach (IEntity waypointEntity : children)
			{
				auto tmpWaypoint = SCR_AIWaypoint.Cast(waypointEntity);
				auto tmpCycle = AIWaypointCycle.Cast(waypointEntity);
					
				if(tmpWaypoint &&  !tmpCycle) 
				{
					Print("waypoint in patrol");
					patrolWaypoints.Insert(AIWaypoint.Cast(waypointEntity));
				}
				if(tmpCycle) 
				{
					Print("waypoint Cycle added");
					cycle = tmpCycle;
				}
			}
			cycle.SetWaypoints(patrolWaypoints);
		}
		
		//
		//add the waypoints
		if(cycle)
		{
		 	agent.AddWaypoint(cycle);
		}
		else if(defend)
		{
			agent.AddWaypoint(defend);
		}
		else if(onBoard)
		{
			agent.AddWaypoint(onBoard);
		}
		
		
		
		//respawn
		if(m_bRespawn)
		{
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(agent);
			if (aiGroup)
			{
				aiGroup.GetOnEmpty().Insert(OnEmpty);
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected event void OnEmpty()
	{
		m_pOnEmptyInvoker.Invoke();
		m_pSpawnedAgent.Deactivate();
		m_pSpawnedAgent = null;
		DoSpawnDefault();
		
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpawned()
	{
		return m_pSpawnedAgent != null;
	}

	//------------------------------------------------------------------------------------------------
	bool DoSpawnDefault()
	{
		return DoSpawn(m_rnDefaultPrefab);
	}
	

	//------------------------------------------------------------------------------------------------
	protected bool VerifyRplComponentPresent()
	{
		if (!m_pRplComponent)
		{
			Print("RLC_AISpawnerComponent does not have a RplComponent attached!");
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTriggerActivate()
	{


		// Spawn when whatever enters this trigger
		if (!IsSpawned())
		{
			if (DoSpawnDefault())
			{
				// Once the group is spawned, in this case let's just disable the
				// trigger, making it a complete one-shot
				GenericEntity.Cast(GetOwner()).Deactivate();
				// Additionally we could just RplComponent.DeleteEntity(GetOwner(), false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		Owner = owner;
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!VerifyRplComponentPresent())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{

		owner.GetTransform(parentVector);
		
		if (m_bSpawnImmediately)
		{
			// Spawning of Replicated items must not happen in EOnInit,
			// we delay the call to happen asap (after EOnInit)
			GetGame().GetCallqueue().CallLater(DoSpawnDefault, 0);
		}
		
		
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{		
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(GetSpawnedAgent());
		if (aiGroup)
		{
			aiGroup.GetOnEmpty().Remove(OnEmpty);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RLC_AISpawnerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~RLC_AISpawnerComponent()
	{
	}

}

