package com.mousebirdconsulting.autotester;

import android.content.Context;
import android.os.Bundle;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import androidx.appcompat.widget.Toolbar;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.mousebirdconsulting.autotester.Framework.MaplyTestResult;

import java.util.ArrayList;


public class ResultActivity extends AppCompatActivity {

	Toolbar toolbar;
	RecyclerView resultsList;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.result_activity);
		toolbar = findViewById(R.id.toolbar);
		resultsList = findViewById(R.id.testsResults_recyclerList);
		configureToolbar();

		try {
			Bundle bundle = getIntent().getExtras();
			@SuppressWarnings("unchecked")	// we'll catch the cast exception if there's a problem
			ArrayList<MaplyTestResult> listResults = (ArrayList<MaplyTestResult>) bundle.getSerializable("arraylist");
			ResultsTestsAdapter adapter = new ResultsTestsAdapter(listResults, this);
			resultsList.setAdapter(adapter);
			resultsList.setLayoutManager(createLayoutManager());
		} catch (Exception ex) {
			System.out.println("Bundle error");
		}
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	private RecyclerView.LayoutManager createLayoutManager() {
		return new LinearLayoutManager(getApplicationContext());
	}

	private void configureToolbar() {
		setSupportActionBar(toolbar);

		//Add home button
		ActionBar actionBar = getSupportActionBar();
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setDisplayShowTitleEnabled(true);
		actionBar.setHomeAsUpIndicator(R.drawable.ic_action_back);
		actionBar.setTitle("Results...");
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {

		int id = item.getItemId();
		if (id == android.R.id.home) {
			finish();
		}
		return true;
	}

	private class ResultsTestsAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
		private ArrayList<MaplyTestResult> results;
		private Context context;

		public ResultsTestsAdapter(ArrayList<MaplyTestResult> results, Context context) {
			this.results = results;
			this.context = context;
			notifyDataSetChanged();
		}

		@Override
		public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
			View view = LayoutInflater.from(context).inflate(R.layout.testlistitemview, parent, false);
			return new ResultsTestHolder(view);
		}

		@Override
		public void onBindViewHolder(RecyclerView.ViewHolder holder, int position) {
			((ResultsTestHolder) holder).bindViewHolder(results.get(position));
			holder.itemView.setBackgroundColor(
				getResources().getColor(
					results.get(position).isPassed()
						? R.color.colorGreen
						: R.color.colorRed));
		}

		@Override
		public int getItemCount() {
			return results.size();
		}


		private class ResultsTestHolder extends RecyclerView.ViewHolder {
			private TextView label;
			private MaplyTestResult result;

			public ResultsTestHolder(View itemView) {
				super(itemView);
				label = (TextView) itemView.findViewById(R.id.testNameLabel);
			}

			public void bindViewHolder(final MaplyTestResult result) {

				label.setText(result.getTestName());
				this.result = result;
				itemView.setOnClickListener(new View.OnClickListener() {
					@Override
					public void onClick(View v) {
						if (!result.isPassed()) {
							Toast.makeText(getApplicationContext(), result.getException().getMessage(), Toast.LENGTH_LONG).show();
						}
					}
				});
			}


		}


	}
}
