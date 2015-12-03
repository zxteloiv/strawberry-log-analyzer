--============================================================================
-- 1. Create the database, and drop it before if it's existed already
------------------------------------------------------------------------------
use master;
go

-- Delete the database first if it was created already
if exists( select * from sys.databases where sys.databases.name = 'Strawberry' )
drop database Strawberry
go

create database Strawberry
go

--============================================================================
-- 2. Create tables in the Stawberry database
------------------------------------------------------------------------------
use Strawberry;
go

-- drop all the tables existed if any
while exists ( select * from Strawberry.sys.tables ) begin
	declare @tname nvarchar(max)
	set @tname = ''
	select @tname = @tname +'['+ name+ '],' from Strawberry.sys.tables
	set @tname = LEFT( @tname, len(@tname) -1 )
	exec( 'drop table ' + @tname)
end
go

create table st_user (
	id			int	primary key IDENTITY(50, 1),	-- 50 is an arbitrary number for
													-- future maintaining
	[password]	nvarchar(15)	not null,
	nickname	nvarchar(15)	not null,
	email		nvarchar(50)	not null unique,
);

go

create table st_log (
	id			int primary key IDENTITY(50, 1),	-- 50 is an arbitrary number for
													-- future maintaining
	uploadTime	datetime,
	[fileName]	nvarchar(260) 	not null unique,
	
	logAlias	nvarchar(50), -- a friendly name to represent this user log file
	size		bigint,
	defXML		xml,
	
	userID		int	foreign key references [st_user]
);

go

create table st_requestState(
	id		int primary key,
	locale	nvarchar(10),	-- language of details
	detail	nvarchar(20)
);
go

create table st_result (
	id			int primary key IDENTITY(50, 1),	-- 50 is an arbitrary number for
													-- future maintaining
	resultXML	xml
);
go

create table st_request (
	id			int primary key IDENTITY(50, 1),	-- 50 is an arbitrary number for
													-- future maintaining
	submitDate	datetime,
	finishDate	datetime,
	
	requestAlias nvarchar(50),	-- a friendly name for recognizing requests
	
	priority	int, -- priority for the request
	
	requestXML	xml,
	
	stateID		int foreign key references [st_requestState] not null,
	resultID	int foreign key references [st_result],
	logID		int foreign key references [st_log] not null,
	userID		int foreign key references [st_user] not null
);
go

insert into st_requestState values( 1, 'en-US', 'Empty' )
insert into st_requestState values( 2, 'en-US', 'Pending' )
insert into st_requestState values( 3, 'en-US', 'Preparing' )
insert into st_requestState values( 4, 'en-US', 'Processing' )
insert into st_requestState values( 5, 'en-US', 'Success' )
insert into st_requestState values( 6, 'en-US', 'Fail' )
insert into st_requestState values( 7, 'en-US', 'Admin Canceled' )
insert into st_requestState values( 8, 'en-US', 'User Canceled' )
go

insert into st_user values('123456', 'testuser', 'testuser@test.com')
go

insert into st_log values('2000-1-1', 'S:\\LogStorage\log_test1.txt', 'my log', 0, '<LogDefinition encoding="gb2312" type="text"><Record FieldsCount="5" StartSymbol="" EndSymbol="\n"><Field StartSymbol="" EndSymbol="\t" trim="true" type="string" alias="user id" /><Field StartSymbol="" EndSymbol="\t" trim="true" type="string" alias="search keywords" /><Field StartSymbol="" EndSymbol="\t" trim="true" type="int" alias="rank" /><Field StartSymbol="" EndSymbol="\t" trim="true" type="int" alias="click time" /><Field StartSymbol="" EndSymbol="\r" trim="true" type="string" alias="date" /></Record></LogDefinition>', 50)
insert into st_log values('2011-6-15', 'S:\\LogStorage\SogouQ.sample', '搜狗日志mini版', 776282, '<LogDefinition encoding="gb2312" type="text"><Record FieldsCount="5" StartSymbol="" EndSymbol=""><Field StartSymbol="" EndSymbol="\t[" trim="true" type="string" alias="用户ID" /><Field StartSymbol="" EndSymbol="]\t" trim="true" type="string" alias="查询词" /><Field StartSymbol="" EndSymbol="\t" trim="true" type="int" alias="排名" /><Field StartSymbol="" EndSymbol="\t" trim="true" type="int" alias="点击顺序号" /><Field StartSymbol="" EndSymbol="\n" trim="true" type="string" alias="URL" /></Record></LogDefinition>', 50)
go

insert into st_request values('2011-5-21', NULL, 'DB_查询重复率随时间分布', 0, '<request><aggregators><!-- Aggregator definitions --><agg id="0"><base_field id="4" /><range_func auto_range="true" type="string" image_name="ExternalRoutes.dll" func_name=""><string case_sensitive="true" number_of_char_to_comp="5" reversed_comp="false" /></range_func><aggregate_func type="count" /><accumulate direction="right" /><condition_fields number=""><field id="3" image_name="ExternalRoutes.dll" func_name="_IntEqualTo@8" params="1" prefered_value="true" /></condition_fields></agg><agg id="1"><base_field id="1" /><range_func auto_range="true" type="string" image_name="ExternalRoutes.dll" func_name=""><string case_sensitive="true" number_of_char_to_comp="-1" reversed_comp="false" /></range_func><aggregate_func type="count" /><accumulate direction="none" /><condition_fields number="" /></agg></aggregators><questions><question id="0" text="查询重复率随日期变化" agg_id="0" has_range_agg="true" has_result_agg="false" save_range_value="true"><range_agg agg_id="1" has_range_agg="false" has_result_agg="false" save_range_count="true" /></question></questions></request>', 2, NULL, 50, 50)
go

