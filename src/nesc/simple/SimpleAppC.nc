configuration SimpleAppC{
}
implementation{ 
	components SimpleC, MainC;

	SimpleC.Boot -> MainC.Boot;
}
